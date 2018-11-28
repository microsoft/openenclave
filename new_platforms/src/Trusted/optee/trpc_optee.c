/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* Licensed under the MIT License. */
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sgx_trts.h>

#include <pta_rpc.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <openenclave/enclave.h>
#include "enclavelibc.h"
#include "sgxoveroptee_enclave.h"
#include "../oeoverintelsgx_t.h"
#include "oeresult.h"

// g_ocallRpcKey relies on the fact that a second thread calling into
// an OPTEE TA blocks until the first caller thread returns to the Rich OS.
static uint32_t g_ocallRpcKey = 0;

void OpteeSetRpcKey(uint32_t rpcKey)
{
    g_ocallRpcKey = rpcKey;
}

static TEE_Result
ExecuteGenericRpc(
	uint32_t param_types, 
	TEE_Param params[TEE_NUM_PARAMS])
{
	static TEE_TASessionHandle sess = TEE_HANDLE_NULL;
	static const TEE_UUID pta_uuid = PTA_RPC_UUID;
	TEE_Result result = TEE_SUCCESS;

	if (sess == TEE_HANDLE_NULL)
	{
		result = TEE_OpenTASession(&pta_uuid, 0, 0, NULL, &sess, NULL);

		if (result != TEE_SUCCESS)
		{
			EMSG("TEE_OpenTASession failed, error %#x\n", result);
			return result;
		}
	}

	result = TEE_InvokeTACommand(sess, 0, PTA_RPC_EXECUTE, param_types, params, NULL);
	
	if (result != TEE_SUCCESS)
	{
		EMSG("TEE_InvokeTACommand failed, error %#x\n", result);
	}

	return result;
}

TEE_Result
TcpsExecuteRPC(
	uint32_t rpcKey,
	uint32_t rpcType,
	const void *rpcInputBuffer,
	uint32_t rpcInputSize,
	void *rpcOutputBuffer,
	uint32_t rpcOutputSize,
	uint32_t *rpcOutputSizeWritten)
{
	uint32_t dummyInput = 0x123456;
	uint32_t dummyOutput = 0x654321;
	TEE_Result result = TEE_SUCCESS;

   	uint32_t pt = TEE_PARAM_TYPES(
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_MEMREF_INPUT,
		TEE_PARAM_TYPE_MEMREF_OUTPUT,
		TEE_PARAM_TYPE_NONE);

	TEE_Param params[TEE_NUM_PARAMS];

	*rpcOutputSizeWritten = 0;

	if (rpcInputBuffer == NULL && rpcInputSize == 0)
	{
		rpcInputBuffer = &dummyInput;
		rpcInputSize = sizeof(dummyInput);
	}
	if (rpcOutputBuffer == NULL && rpcOutputSize == 0)
	{
		rpcOutputBuffer = &dummyOutput;
		rpcOutputSize = sizeof(dummyOutput);
	}

    if (rpcKey == 0)
    {
        EMSG("rpcKey is 0");
        result = OE_FAILURE;
        goto Done;
    }

	memset(params, 0, sizeof(params));

	params[0].value.a = rpcType;
	params[0].value.b = rpcKey;

	params[1].memref.buffer = (void *)rpcInputBuffer;
	params[1].memref.size = rpcInputSize;

	params[2].memref.buffer = rpcOutputBuffer;
	params[2].memref.size = rpcOutputSize;

	result = ExecuteGenericRpc(pt, params);

	if (result != TEE_SUCCESS)
	{
        EMSG("ExecuteGenericRpc failed, rpcType = %u, rpcKey = %u", rpcType, rpcKey);
		goto Done;
	}

	if (rpcOutputSizeWritten != NULL)
	{
		*rpcOutputSizeWritten = params[2].memref.size;
		assert(*rpcOutputSizeWritten <= rpcOutputSize);
	}

Done:
    /*
	FMSG("RPC key = %#x, type %u -> returning TEE_Result = %#x, wrote %u/%u bytes",
		rpcKey, rpcType, result, *rpcOutputSizeWritten, rpcOutputSize);
        */

	return result;
}

TEE_Result
TcpsGetParameters(
    uint32_t    param_types,
    TEE_Param   params[TEE_NUM_PARAMS],
    uint32_t    *rpcKey,
    void        **in_buffer,
    uint32_t    *in_buffer_size,
    void        **out_buffer,
    uint32_t    *out_buffer_size,
    uint32_t    **bytes_written)
{
    TEE_Result result = TEE_SUCCESS;
    uint32_t paramIndex;

    *rpcKey = 0;
    *in_buffer = NULL;
    *in_buffer_size = 0;
    *out_buffer = NULL;
    *out_buffer_size = 0;
    *bytes_written = NULL;

    // first parameter is always the RPC key.
    if (TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_VALUE_INPUT)
    {
        EMSG("TcpsGetParameters: incorrect parameter 0 type: %#x, expected %#x\n",
             TEE_PARAM_TYPE_GET(param_types, 0),
             TEE_PARAM_TYPE_VALUE_INPUT);
        result = TEE_ERROR_BAD_PARAMETERS;
        goto Done;
    }

    paramIndex = 0;
    *rpcKey = params[paramIndex].value.a;
    paramIndex++;

    if (TEE_PARAM_TYPE_GET(param_types, paramIndex) == TEE_PARAM_TYPE_MEMREF_INPUT)
    {
        *in_buffer      = params[paramIndex].memref.buffer;
        *in_buffer_size = params[paramIndex].memref.size;
        paramIndex++;
    }

    if (TEE_PARAM_TYPE_GET(param_types, paramIndex) == TEE_PARAM_TYPE_MEMREF_INOUT)
    {
        *out_buffer      = params[paramIndex].memref.buffer;
        *out_buffer_size = params[paramIndex].memref.size;
        *bytes_written   = &params[paramIndex].memref.size;
    }

Done:

    /*
    FMSG("rpcKey = %#x, in = 0x%p size %#x, out = 0x%p size %#x, TEE_Result = %#x\n",
         *rpcKey,
         *in_buffer,
         *in_buffer_size,
         *out_buffer,
         *out_buffer_size,
         result);
         */

    return result;
}

sgx_status_t optee_ocall(
    const unsigned int id,
    const void* inputBuffer,
    size_t inputBufferSize,
    void* outputBuffer,
    size_t outputBufferSize)
{
    uint32_t bytesWritten;
    uint32_t inputSize = (uint32_t)inputBufferSize;
    uint32_t outputSize = (uint32_t)outputBufferSize;

    /*
    FMSG("optee_ocall: invoking id %d with input size %u, output size %u, key = %#x\n",
        id, inputSize, outputSize, g_ocallRpcKey);
        */

    TEE_Result result = TcpsExecuteRPC(g_ocallRpcKey,
                                       id,
                                       inputBuffer,
                                       inputSize,
                                       outputBuffer,
                                       outputSize,
                                       &bytesWritten);

    if (result != TEE_SUCCESS) {
        EMSG("RPC failed with result %#x, ocall = %u, g_ocallRpcKey = %u", result, id, g_ocallRpcKey);
        return SGX_ERROR_UNEXPECTED;
    }

    // FMSG("optee_ocall: succeeded with %u bytes written\n", bytesWritten);

    return SGX_SUCCESS;
}

int sgx_is_within_enclave(const void* addr, size_t size)
{
    TEE_Result result = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_SECURE, (void*)addr, size);
    return (result == TEE_SUCCESS);
}

int sgx_is_outside_enclave(const void* addr, size_t size)
{
#ifdef OE_SIMULATE_OPTEE
    /* Temporary workaround. */
    return TRUE;
#else
    return !sgx_is_within_enclave(addr, size);
#endif
}

sgx_status_t
sgx_optee_ocall(
    const unsigned int index,
    void* buffer,
    size_t bufferSize)
{
    sgx_status_t sgxStatus;

    /* Use the same buffer as the input and output buffer.
     * It will be modified in place.
     */
    sgxStatus = optee_ocall(index, buffer, bufferSize, buffer, bufferSize);

    return sgxStatus;
}

typedef sgx_status_t (*ecall_fcn_t)(void* buffer);

extern const struct {
    size_t nr_ecall;
    struct {ecall_fcn_t call_addr; uint8_t is_priv;} ecall_table[1];
} g_ecall_table;

TEE_Result
TcpsEcallDemux(
    void *sess_ctx,
    uint32_t cmd_id,
    uint32_t param_types,
    TEE_Param params[4])
{
    const uint8_t* in_buffer;
    uint32_t in_buffer_size;
    uint8_t* out_buffer;
    uint32_t out_buffer_size;
    uint32_t* bytes_written_ptr;
    uint32_t rpcKey;
    TEE_Result result;
    sgx_status_t sgxStatus = SGX_SUCCESS;

    OE_UNUSED(sess_ctx);

    if (cmd_id >= g_ecall_table.nr_ecall) {
        return SGX_ERROR_INVALID_PARAMETER;
    }

    result = TcpsGetParameters(param_types, params, &rpcKey, (void**)&in_buffer, &in_buffer_size,
                               (void**)&out_buffer, &out_buffer_size, &bytes_written_ptr);
    if (result != TEE_SUCCESS) {
        EMSG("Command %d: couldn't get parameters", cmd_id);
        return result;
    }

    OpteeSetRpcKey(rpcKey);

    if (in_buffer_size != out_buffer_size) {
        result = TEE_ERROR_BAD_PARAMETERS;
        goto Done;
    }

    /* Copy input buffer to output buffer. */
    memcpy(out_buffer, in_buffer, out_buffer_size);

    /* Now invoke the ecall handler on the output buffer. */
    sgxStatus = g_ecall_table.ecall_table[cmd_id].call_addr(out_buffer);

    result = (sgxStatus == SGX_SUCCESS) ? TEE_SUCCESS : TEE_ERROR_GENERIC;

Done:
    return result;
}

#include <openenclave/edger8r/enclave.h>
extern oe_ecall_func_t __oe_ecalls_table[];
extern size_t __oe_ecalls_table_size;

TEE_Result
_oe_EcallDemux(
    void *sess_ctx,
    uint32_t cmd_id,
    uint32_t param_types,
    TEE_Param params[4])
{
    const uint8_t* in_buffer;
    uint32_t in_buffer_size;
    uint8_t* out_buffer;
    uint32_t out_buffer_size;
    uint32_t* bytes_written_ptr;
    uint32_t rpcKey;
    TEE_Result result;

    OE_UNUSED(sess_ctx);

    if (cmd_id >= __oe_ecalls_table_size)
    {
        return SGX_ERROR_INVALID_PARAMETER;
    }

    result = TcpsGetParameters(param_types, params, &rpcKey, (void**)&in_buffer, &in_buffer_size,
        (void**)&out_buffer, &out_buffer_size, &bytes_written_ptr);
    if (result != TEE_SUCCESS) {
        EMSG("Command %d: couldn't get parameters", cmd_id);
        return result;
    }

    OpteeSetRpcKey(rpcKey);

    /* Copy the input buffer into enclave memory. */
    uint8_t* enclave_in_buffer = oe_malloc(in_buffer_size);
    if (enclave_in_buffer == NULL) {
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    memcpy(enclave_in_buffer, in_buffer, in_buffer_size);

    uint8_t* enclave_out_buffer = oe_malloc(out_buffer_size);
    if (enclave_out_buffer == NULL) {
        oe_free(enclave_in_buffer);
        return TEE_ERROR_OUT_OF_MEMORY;
    }

    /* Now invoke the ecall handler on the enclave buffers. */
    size_t bytes_written = 0;
    __oe_ecalls_table[cmd_id](
        enclave_in_buffer,
        in_buffer_size,
        enclave_out_buffer,
        out_buffer_size,
        &bytes_written);

    memcpy(out_buffer, enclave_out_buffer, bytes_written);
    *bytes_written_ptr = bytes_written;
    oe_free(enclave_in_buffer);
    oe_free(enclave_out_buffer);

    return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is created.
 * This is the first call in the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
    FMSG("created");
    return TEE_SUCCESS;
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(
    uint32_t param_types,
    TEE_Param params[4],
    void **sess_ctx)
{
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types) {
        EMSG("error bad param types");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    OE_UNUSED(params);

    *sess_ctx = NULL;

    FMSG("session opened");
    return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx holds that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the parameters
 * come from the normal world app.
 */
TEE_Result TA_InvokeCommandEntryPoint(
    void *sess_ctx,
    uint32_t cmd_id,
    uint32_t param_types,
    TEE_Param params[4])
{
    if (cmd_id < V2_FUNCTION_ID_OFFSET) {
        /* Handle ECALL generated by sgx_edger8r. */
        return TcpsEcallDemux(sess_ctx, cmd_id, param_types, params);
    }

    /* Handle ECALL generated by oeedger8r. */
    return _oe_EcallDemux(sess_ctx, cmd_id - V2_FUNCTION_ID_OFFSET, param_types, params);
}

/*
 * Called when a session is closed. sess_ctx holds the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void *sess_ctx)
{
    OE_UNUSED(sess_ctx);
    FMSG("session closed");
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
    FMSG("destroyed");
}

oe_result_t oe_call_host_function(
    size_t function_id,
    const void* input_buffer,
    size_t input_buffer_size,
    void* output_buffer,
    size_t output_buffer_size,
    size_t* output_bytes_written)
{
    if (input_buffer_size > UINT32_MAX || output_buffer_size > UINT32_MAX) {
        return OE_INVALID_PARAMETER;
    }

    uint32_t bytesWritten;
    TEE_Result result = TcpsExecuteRPC(
        g_ocallRpcKey,
        V2_FUNCTION_ID_OFFSET + function_id,
        input_buffer,
        (uint32_t)input_buffer_size,
        output_buffer,
        (uint32_t)output_buffer_size,
        &bytesWritten);

    if (result != TEE_SUCCESS) {
        return SGX_ERROR_UNEXPECTED;
    }

    *output_bytes_written = bytesWritten;
    return SGX_SUCCESS;
}
