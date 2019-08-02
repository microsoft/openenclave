/*
 *  This file is auto generated by oeedger8r. DO NOT EDIT.
 */
#ifndef EDGER8R_SWITCHLESS_ARGS_H
#define EDGER8R_SWITCHLESS_ARGS_H

#include <stdint.h>
#include <stdlib.h> /* for wchar_t */

/* #include <errno.h> - Errno propagation not enabled so not included. */

#include <openenclave/bits/result.h>

/**** User includes. ****/
#include "openenclave/edger8r/switchless.h"

/**** User defined types in EDL. ****/
typedef struct addition_args
{
    int arg1;
    int arg2;
    int sum;
} addition_args;

/**** ECALL marshalling structs. ****/
typedef struct _standard_enc_sum_args_t
{
    oe_result_t _result;
    int _retval;
    int arg1;
    int arg2;
} standard_enc_sum_args_t;

typedef struct _synchronous_switchless_enc_sum_args_t
{
    oe_result_t _result;
    int _retval;
    int arg1;
    int arg2;
} synchronous_switchless_enc_sum_args_t;

typedef struct _batch_enc_sum_args_t
{
    oe_result_t _result;
    addition_args* args;
    size_t count;
} batch_enc_sum_args_t;

typedef struct _enc_test_args_t
{
    oe_result_t _result;
    int _retval;
    oe_switchless_t* arg;
    int type;
    addition_args* args;
    size_t count;
} enc_test_args_t;

/**** OCALL marshalling structs. ****/
typedef struct _standard_host_sum_args_t
{
    oe_result_t _result;
    int _retval;
    int arg1;
    int arg2;
} standard_host_sum_args_t;

typedef struct _synchronous_switchless_host_sum_args_t
{
    oe_result_t _result;
    int _retval;
    int arg1;
    int arg2;
} synchronous_switchless_host_sum_args_t;

typedef struct _batch_host_sum_args_t
{
    oe_result_t _result;
    addition_args* args;
    size_t count;
} batch_host_sum_args_t;

/**** Trusted function IDs ****/
enum
{
    switchless_fcn_id_standard_enc_sum = 0,
    switchless_fcn_id_synchronous_switchless_enc_sum = 1,
    switchless_fcn_id_batch_enc_sum = 2,
    switchless_fcn_id_enc_test = 3,
    switchless_fcn_id_trusted_call_id_max = OE_ENUM_MAX
};

/**** Untrusted function IDs. ****/
enum
{
    switchless_fcn_id_standard_host_sum = 0,
    switchless_fcn_id_synchronous_switchless_host_sum = 1,
    switchless_fcn_id_batch_host_sum = 2,
    switchless_fcn_id_untrusted_call_max = OE_ENUM_MAX
};

#endif // EDGER8R_SWITCHLESS_ARGS_H
