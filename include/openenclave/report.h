// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_REPORT_H
#define _OE_REPORT_H

#include "defs.h"

OE_EXTERNC_BEGIN

/*
 * Options passed to GetReport functions on host and enclave.
 * Default value (0) is local attestation.
 */
#define OE_REPORT_OPTIONS_REMOTE_ATTESTATION 0x00000001

#define OE_REPORT_DATA_SIZE 64

OE_EXTERNC_END

#endif /* _OE_REPORT_H */
