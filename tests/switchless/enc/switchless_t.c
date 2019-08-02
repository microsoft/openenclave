/*
 *  This file is auto generated by oeedger8r. DO NOT EDIT.
 */
#include "switchless_t.h"

#include <openenclave/edger8r/enclave.h>

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

OE_EXTERNC_BEGIN

/**** ECALL functions. ****/

void ecall_standard_enc_sum(
    uint8_t* input_buffer,
    size_t input_buffer_size,
    uint8_t* output_buffer,
    size_t output_buffer_size,
    size_t* output_bytes_written)
{
    oe_result_t _result = OE_FAILURE;

    /* Prepare parameters. */
    standard_enc_sum_args_t* pargs_in = (standard_enc_sum_args_t*)input_buffer;
    standard_enc_sum_args_t* pargs_out = (standard_enc_sum_args_t*)output_buffer;

    size_t input_buffer_offset = 0;
    size_t output_buffer_offset = 0;
    OE_ADD_SIZE(input_buffer_offset, sizeof(*pargs_in));
    OE_ADD_SIZE(output_buffer_offset, sizeof(*pargs_out));

    /* Make sure input and output buffers lie within the enclave. */
    if (!input_buffer || !oe_is_within_enclave(input_buffer, input_buffer_size))
        goto done;

    if (!output_buffer || !oe_is_within_enclave(output_buffer, output_buffer_size))
        goto done;

    /* Set in and in-out pointers. */
    /* There were no in nor in-out parameters. */

    /* Set out and in-out pointers. */
    /* In-out parameters are copied to output buffer. */
    /* There were no out nor in-out parameters. */

    /* Check that in/in-out strings are null terminated. */
    /* There were no in nor in-out string parameters. */

    /* lfence after checks. */
    oe_lfence();

    /* Call user function. */
    pargs_out->_retval = standard_enc_sum(
        pargs_in->arg1,
        pargs_in->arg2);

    /* Success. */
    _result = OE_OK;
    *output_bytes_written = output_buffer_offset;

done:
    if (pargs_out && output_buffer_size >= sizeof(*pargs_out))
        pargs_out->_result = _result;
}

void ecall_synchronous_switchless_enc_sum(
    uint8_t* input_buffer,
    size_t input_buffer_size,
    uint8_t* output_buffer,
    size_t output_buffer_size,
    size_t* output_bytes_written)
{
    oe_result_t _result = OE_FAILURE;

    /* Prepare parameters. */
    synchronous_switchless_enc_sum_args_t* pargs_in = (synchronous_switchless_enc_sum_args_t*)input_buffer;
    synchronous_switchless_enc_sum_args_t* pargs_out = (synchronous_switchless_enc_sum_args_t*)output_buffer;

    size_t input_buffer_offset = 0;
    size_t output_buffer_offset = 0;
    OE_ADD_SIZE(input_buffer_offset, sizeof(*pargs_in));
    OE_ADD_SIZE(output_buffer_offset, sizeof(*pargs_out));

    /* Make sure input and output buffers lie within the enclave. */
    if (!input_buffer || !oe_is_within_enclave(input_buffer, input_buffer_size))
        goto done;

    if (!output_buffer || !oe_is_within_enclave(output_buffer, output_buffer_size))
        goto done;

    /* Set in and in-out pointers. */
    /* There were no in nor in-out parameters. */

    /* Set out and in-out pointers. */
    /* In-out parameters are copied to output buffer. */
    /* There were no out nor in-out parameters. */

    /* Check that in/in-out strings are null terminated. */
    /* There were no in nor in-out string parameters. */

    /* lfence after checks. */
    oe_lfence();

    /* Call user function. */
    pargs_out->_retval = synchronous_switchless_enc_sum(
        pargs_in->arg1,
        pargs_in->arg2);

    /* Success. */
    _result = OE_OK;
    *output_bytes_written = output_buffer_offset;

done:
    if (pargs_out && output_buffer_size >= sizeof(*pargs_out))
        pargs_out->_result = _result;
}

void ecall_batch_enc_sum(
    uint8_t* input_buffer,
    size_t input_buffer_size,
    uint8_t* output_buffer,
    size_t output_buffer_size,
    size_t* output_bytes_written)
{
    oe_result_t _result = OE_FAILURE;

    /* Prepare parameters. */
    batch_enc_sum_args_t* pargs_in = (batch_enc_sum_args_t*)input_buffer;
    batch_enc_sum_args_t* pargs_out = (batch_enc_sum_args_t*)output_buffer;

    size_t input_buffer_offset = 0;
    size_t output_buffer_offset = 0;
    OE_ADD_SIZE(input_buffer_offset, sizeof(*pargs_in));
    OE_ADD_SIZE(output_buffer_offset, sizeof(*pargs_out));

    /* Make sure input and output buffers lie within the enclave. */
    if (!input_buffer || !oe_is_within_enclave(input_buffer, input_buffer_size))
        goto done;

    if (!output_buffer || !oe_is_within_enclave(output_buffer, output_buffer_size))
        goto done;

    /* Set in and in-out pointers. */
    if (pargs_in->args)
        OE_SET_IN_OUT_POINTER(args, (pargs_in->count * sizeof(addition_args)), addition_args*);

    /* Set out and in-out pointers. */
    /* In-out parameters are copied to output buffer. */
    if (pargs_in->args)
        OE_COPY_AND_SET_IN_OUT_POINTER(args, (pargs_in->count * sizeof(addition_args)), addition_args*);

    /* Check that in/in-out strings are null terminated. */
    /* There were no in nor in-out string parameters. */

    /* lfence after checks. */
    oe_lfence();

    /* Call user function. */
    batch_enc_sum(
        pargs_in->args,
        pargs_in->count);

    /* Success. */
    _result = OE_OK;
    *output_bytes_written = output_buffer_offset;

done:
    if (pargs_out && output_buffer_size >= sizeof(*pargs_out))
        pargs_out->_result = _result;
}

void ecall_enc_test(
    uint8_t* input_buffer,
    size_t input_buffer_size,
    uint8_t* output_buffer,
    size_t output_buffer_size,
    size_t* output_bytes_written)
{
    oe_result_t _result = OE_FAILURE;

    /* Prepare parameters. */
    enc_test_args_t* pargs_in = (enc_test_args_t*)input_buffer;
    enc_test_args_t* pargs_out = (enc_test_args_t*)output_buffer;

    size_t input_buffer_offset = 0;
    size_t output_buffer_offset = 0;
    OE_ADD_SIZE(input_buffer_offset, sizeof(*pargs_in));
    OE_ADD_SIZE(output_buffer_offset, sizeof(*pargs_out));

    /* Make sure input and output buffers lie within the enclave. */
    if (!input_buffer || !oe_is_within_enclave(input_buffer, input_buffer_size))
        goto done;

    if (!output_buffer || !oe_is_within_enclave(output_buffer, output_buffer_size))
        goto done;

    /* Set in and in-out pointers. */
    if (pargs_in->args)
        OE_SET_IN_OUT_POINTER(args, (pargs_in->count * sizeof(addition_args)), addition_args*);

    /* Set out and in-out pointers. */
    /* In-out parameters are copied to output buffer. */
    if (pargs_in->args)
        OE_COPY_AND_SET_IN_OUT_POINTER(args, (pargs_in->count * sizeof(addition_args)), addition_args*);

    /* Check that in/in-out strings are null terminated. */
    /* There were no in nor in-out string parameters. */

    /* lfence after checks. */
    oe_lfence();

    /* Call user function. */
    pargs_out->_retval = enc_test(
        pargs_in->arg,
        pargs_in->type,
        pargs_in->args,
        pargs_in->count);

    /* Success. */
    _result = OE_OK;
    *output_bytes_written = output_buffer_offset;

done:
    if (pargs_out && output_buffer_size >= sizeof(*pargs_out))
        pargs_out->_result = _result;
}

/**** ECALL function table. ****/

oe_ecall_func_t __oe_ecalls_table[] = {
    (oe_ecall_func_t) ecall_standard_enc_sum,
    (oe_ecall_func_t) ecall_synchronous_switchless_enc_sum,
    (oe_ecall_func_t) ecall_batch_enc_sum,
    (oe_ecall_func_t) ecall_enc_test
};

size_t __oe_ecalls_table_size = OE_COUNTOF(__oe_ecalls_table);

/**** OCALL function wrappers. ****/

oe_result_t standard_host_sum(
    int* _retval,
    int arg1,
    int arg2)
{
    oe_result_t _result = OE_FAILURE;

    /* If the enclave is in crashing/crashed status, new OCALL should fail
       immediately. */
    if (oe_get_enclave_status() != OE_OK)
        return oe_get_enclave_status();

    /* Marshalling struct. */
    standard_host_sum_args_t _args, *_pargs_in = NULL, *_pargs_out = NULL;

    /* Marshalling buffer and sizes. */
    size_t _input_buffer_size = 0;
    size_t _output_buffer_size = 0;
    size_t _total_buffer_size = 0;
    uint8_t* _buffer = NULL;
    uint8_t* _input_buffer = NULL;
    uint8_t* _output_buffer = NULL;
    size_t _input_buffer_offset = 0;
    size_t _output_buffer_offset = 0;
    size_t _output_bytes_written = 0;

    /* Fill marshalling struct. */
    memset(&_args, 0, sizeof(_args));
    _args.arg1 = arg1;
    _args.arg2 = arg2;

    /* Compute input buffer size. Include in and in-out parameters. */
    OE_ADD_SIZE(_input_buffer_size, sizeof(standard_host_sum_args_t));
    /* There were no corresponding parameters. */
    
    /* Compute output buffer size. Include out and in-out parameters. */
    OE_ADD_SIZE(_output_buffer_size, sizeof(standard_host_sum_args_t));
    /* There were no corresponding parameters. */
    
    /* Allocate marshalling buffer. */
    _total_buffer_size = _input_buffer_size;
    OE_ADD_SIZE(_total_buffer_size, _output_buffer_size);
    _buffer = (uint8_t*)oe_allocate_ocall_buffer(_total_buffer_size);
    _input_buffer = _buffer;
    _output_buffer = _input_buffer + _input_buffer_size;
    if (_buffer == NULL)
    {
        _result = OE_OUT_OF_MEMORY;
        goto done;
    }
    
    /* Serialize buffer inputs (in and in-out parameters). */
    _pargs_in = (standard_host_sum_args_t*)_input_buffer;
    OE_ADD_SIZE(_input_buffer_offset, sizeof(*_pargs_in));
    /* There were no in nor in-out parameters. */
    
    /* Copy args structure (now filled) to input buffer. */
    memcpy(_pargs_in, &_args, sizeof(*_pargs_in));

    /* Call host function. */
    if ((_result = oe_call_host_function(
             switchless_fcn_id_standard_host_sum,
             _input_buffer,
             _input_buffer_size,
             _output_buffer,
             _output_buffer_size,
             &_output_bytes_written)) != OE_OK)
        goto done;

    /* Setup output arg struct pointer. */
    _pargs_out = (standard_host_sum_args_t*)_output_buffer;
    OE_ADD_SIZE(_output_buffer_offset, sizeof(*_pargs_out));
    
    /* Check if the call succeeded. */
    if ((_result = _pargs_out->_result) != OE_OK)
        goto done;
    
    /* Currently exactly _output_buffer_size bytes must be written. */
    if (_output_bytes_written != _output_buffer_size)
    {
        _result = OE_FAILURE;
        goto done;
    }
    
    /* Unmarshal return value and out, in-out parameters. */
    *_retval = _pargs_out->_retval;

    /* Retrieve propagated errno from OCALL. */
    /* Errno propagation not enabled. */

    _result = OE_OK;

done:
    if (_buffer)
        oe_free_ocall_buffer(_buffer);
    return _result;
}

oe_result_t synchronous_switchless_host_sum(
#if _USE_SWITCHLESS
    oe_switchless_t* switchless,
#endif
    int* _retval,
    int arg1,
    int arg2)
{
    oe_result_t _result = OE_FAILURE;

    /* If the enclave is in crashing/crashed status, new OCALL should fail
       immediately. */
    if (oe_get_enclave_status() != OE_OK)
        return oe_get_enclave_status();

    /* Marshalling struct. */
    synchronous_switchless_host_sum_args_t _args, *_pargs_in = NULL, *_pargs_out = NULL;

    /* Marshalling buffer and sizes. */
    size_t _input_buffer_size = 0;
    size_t _output_buffer_size = 0;
    size_t _total_buffer_size = 0;
    uint8_t* _buffer = NULL;
    uint8_t* _input_buffer = NULL;
    uint8_t* _output_buffer = NULL;
    size_t _input_buffer_offset = 0;
    size_t _output_buffer_offset = 0;
    size_t _output_bytes_written = 0;

    /* Fill marshalling struct. */
    memset(&_args, 0, sizeof(_args));
    _args.arg1 = arg1;
    _args.arg2 = arg2;

    /* Compute input buffer size. Include in and in-out parameters. */
    OE_ADD_SIZE(_input_buffer_size, sizeof(synchronous_switchless_host_sum_args_t));
    /* There were no corresponding parameters. */
    
    /* Compute output buffer size. Include out and in-out parameters. */
    OE_ADD_SIZE(_output_buffer_size, sizeof(synchronous_switchless_host_sum_args_t));
    /* There were no corresponding parameters. */
    
    /* Allocate marshalling buffer. */
    _total_buffer_size = _input_buffer_size;
    OE_ADD_SIZE(_total_buffer_size, _output_buffer_size);
    _buffer = (uint8_t*)oe_allocate_ocall_buffer(_total_buffer_size);
    _input_buffer = _buffer;
    _output_buffer = _input_buffer + _input_buffer_size;
    if (_buffer == NULL)
    {
        _result = OE_OUT_OF_MEMORY;
        goto done;
    }
    
    /* Serialize buffer inputs (in and in-out parameters). */
    _pargs_in = (synchronous_switchless_host_sum_args_t*)_input_buffer;
    OE_ADD_SIZE(_input_buffer_offset, sizeof(*_pargs_in));
    /* There were no in nor in-out parameters. */
    
    /* Copy args structure (now filled) to input buffer. */
    memcpy(_pargs_in, &_args, sizeof(*_pargs_in));

    /* Call host function. */
#if _USE_SWITCHLESS
    if ((_result = oe_switchless_call_host_function(
             switchless,
#else
    if ((_result = oe_call_host_function(
#endif
             switchless_fcn_id_synchronous_switchless_host_sum,
             _input_buffer,
             _input_buffer_size,
             _output_buffer,
             _output_buffer_size,
             &_output_bytes_written)) != OE_OK)
        goto done;

    /* Setup output arg struct pointer. */
    _pargs_out = (synchronous_switchless_host_sum_args_t*)_output_buffer;
    OE_ADD_SIZE(_output_buffer_offset, sizeof(*_pargs_out));
    
    /* Check if the call succeeded. */
    if ((_result = _pargs_out->_result) != OE_OK)
        goto done;
    
    /* Currently exactly _output_buffer_size bytes must be written. */
    if (_output_bytes_written != _output_buffer_size)
    {
        _result = OE_FAILURE;
        goto done;
    }
    
    /* Unmarshal return value and out, in-out parameters. */
    *_retval = _pargs_out->_retval;

    /* Retrieve propagated errno from OCALL. */
    /* Errno propagation not enabled. */

    _result = OE_OK;

done:
    if (_buffer)
        oe_free_ocall_buffer(_buffer);
    return _result;
}

oe_result_t batch_host_sum(
    addition_args* args,
    size_t count)
{
    oe_result_t _result = OE_FAILURE;

    /* If the enclave is in crashing/crashed status, new OCALL should fail
       immediately. */
    if (oe_get_enclave_status() != OE_OK)
        return oe_get_enclave_status();

    /* Marshalling struct. */
    batch_host_sum_args_t _args, *_pargs_in = NULL, *_pargs_out = NULL;

    /* Marshalling buffer and sizes. */
    size_t _input_buffer_size = 0;
    size_t _output_buffer_size = 0;
    size_t _total_buffer_size = 0;
    uint8_t* _buffer = NULL;
    uint8_t* _input_buffer = NULL;
    uint8_t* _output_buffer = NULL;
    size_t _input_buffer_offset = 0;
    size_t _output_buffer_offset = 0;
    size_t _output_bytes_written = 0;

    /* Fill marshalling struct. */
    memset(&_args, 0, sizeof(_args));
    _args.args = (addition_args*)args;
    _args.count = count;

    /* Compute input buffer size. Include in and in-out parameters. */
    OE_ADD_SIZE(_input_buffer_size, sizeof(batch_host_sum_args_t));
    if (args)
        OE_ADD_SIZE(_input_buffer_size, (_args.count * sizeof(addition_args)));
    
    /* Compute output buffer size. Include out and in-out parameters. */
    OE_ADD_SIZE(_output_buffer_size, sizeof(batch_host_sum_args_t));
    if (args)
        OE_ADD_SIZE(_output_buffer_size, (_args.count * sizeof(addition_args)));
    
    /* Allocate marshalling buffer. */
    _total_buffer_size = _input_buffer_size;
    OE_ADD_SIZE(_total_buffer_size, _output_buffer_size);
    _buffer = (uint8_t*)oe_allocate_ocall_buffer(_total_buffer_size);
    _input_buffer = _buffer;
    _output_buffer = _input_buffer + _input_buffer_size;
    if (_buffer == NULL)
    {
        _result = OE_OUT_OF_MEMORY;
        goto done;
    }
    
    /* Serialize buffer inputs (in and in-out parameters). */
    _pargs_in = (batch_host_sum_args_t*)_input_buffer;
    OE_ADD_SIZE(_input_buffer_offset, sizeof(*_pargs_in));
    if (args)
        OE_WRITE_IN_OUT_PARAM(args, (_args.count * sizeof(addition_args)), addition_args*);
    
    /* Copy args structure (now filled) to input buffer. */
    memcpy(_pargs_in, &_args, sizeof(*_pargs_in));

    /* Call host function. */
    if ((_result = oe_call_host_function(
             switchless_fcn_id_batch_host_sum,
             _input_buffer,
             _input_buffer_size,
             _output_buffer,
             _output_buffer_size,
             &_output_bytes_written)) != OE_OK)
        goto done;

    /* Setup output arg struct pointer. */
    _pargs_out = (batch_host_sum_args_t*)_output_buffer;
    OE_ADD_SIZE(_output_buffer_offset, sizeof(*_pargs_out));
    
    /* Check if the call succeeded. */
    if ((_result = _pargs_out->_result) != OE_OK)
        goto done;
    
    /* Currently exactly _output_buffer_size bytes must be written. */
    if (_output_bytes_written != _output_buffer_size)
    {
        _result = OE_FAILURE;
        goto done;
    }
    
    /* Unmarshal return value and out, in-out parameters. */
    /* No return value. */
    OE_READ_IN_OUT_PARAM(args, (size_t)((_args.count * sizeof(addition_args))));

    /* Retrieve propagated errno from OCALL. */
    /* Errno propagation not enabled. */

    _result = OE_OK;

done:
    if (_buffer)
        oe_free_ocall_buffer(_buffer);
    return _result;
}

OE_EXTERNC_END
