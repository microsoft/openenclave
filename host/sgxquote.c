// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#if defined(OE_USE_LIBSGX)

#include "sgxquote.h"
#include <sgx_ql_oe_wrapper.h>

// Check consistency with OE definition.
OE_STATIC_ASSERT(sizeof(sgx_target_info_t) == 512);
OE_STATIC_ASSERT(sizeof(sgx_report_t) == 432);

oe_result_t oe_sgx_qe_get_target_info(uint8_t* target_info)
{
    quote3_error_t err = sgx_qe_get_target_info((sgx_target_info_t*)target_info);
    return (err == SGX_QL_SUCCESS) ? OE_OK : OE_PLATFORM_ERROR;
}

oe_result_t oe_sgx_qe_get_quote_size(uint32_t* quote_size)
{
    quote3_error_t err = sgx_qe_get_quote_size(quote_size);
    return (err == SGX_QL_SUCCESS) ? OE_OK : OE_PLATFORM_ERROR;
}

oe_result_t oe_sgx_qe_get_quote(
    uint8_t* report,
    uint32_t quote_size,
    uint8_t* quote)
{
    quote3_error_t err =
        sgx_qe_get_quote((sgx_report_t*)report, quote_size, quote);
    return (err == SGX_QL_SUCCESS) ? OE_OK : OE_PLATFORM_ERROR;
}

#endif
