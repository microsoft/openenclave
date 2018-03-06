// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/bits/enclavelibc.h>
#include <openenclave/enclave.h>
#include "file_t.h"

OE_EXTERNC int TestReadFile(const char* path, unsigned int* checksum)
{
    int rc = -1;
    MY_FILE* is = NULL;
    const size_t bufSize = 32;
    unsigned char buf[bufSize];
    OE_Result r;

    if (!path || !checksum)
        goto done;

    if ((r = Fopen(&is, path, "rb")) != OE_OK)
        goto done;

    size_t n;
    while ((r = Fread(&n, buf, bufSize, is)) == OE_OK && n > 0)
    {
        for (size_t i = 0; i < n; i++)
            (*checksum) += buf[i];
    }

    if (r != OE_OK)
        goto done;

    rc = 0;

done:

    if (is)
    {
        int ret;
        if ((r = Fclose(&ret, is)) != OE_OK)
            rc = -1;
    }

    return rc;
}
