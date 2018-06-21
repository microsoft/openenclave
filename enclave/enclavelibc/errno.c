// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <errno.h>
#include <openenclave/enclave.h>
#include <openenclave/internal/sgxtypes.h>

int* __oe_errno_location()
{
    TD* td = (TD*)oe_get_thread_data();
    return &td->linux_errno;
}
