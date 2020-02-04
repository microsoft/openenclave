// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/host.h>
#include <stdio.h>

int open_enclave(int argc, const char* argv[]);
int close_enclave();
int call_enclave(char *input_msg, char *enclave_msg, unsigned int enclave_msg_size);

int main(int argc, const char* argv[])
{
    int result = open_enclave(argc, argv);
    if (result != 0)
    {
        return result;
    }
    char* enclaveMessage = (char*)malloc(512 * sizeof(char));
    result = call_enclave("\"Process In Enclave\"", enclaveMessage, 512);
    if (result != 0)
    {
        return result;
    }

    result = close_enclave();
    return result;
}
