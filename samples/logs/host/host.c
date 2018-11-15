// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/host.h>
#include <stdio.h>

// Include the untrusted logs header that is generated
// during the build. This file is generated by calling the
// sdk tool oeedger8r against the logs.edl file.
#include "logs_u.h"

bool check_simulate_opt(int* argc, const char* argv[])
{
    for (int i = 0; i < *argc; i++)
    {
        if (strcmp(argv[i], "--simulate") == 0)
        {
            fprintf(stdout, "Running in simulation mode\n");
            memmove(&argv[i], &argv[i + 1], (*argc - i) * sizeof(char*));
            (*argc)--;
            return true;
        }
    }
    return false;
}

int main(int argc, const char* argv[])
{
    oe_result_t result;
    int ret = 1;
    oe_enclave_t* enclave = NULL;

    uint32_t flags = OE_ENCLAVE_FLAG_DEBUG;

    if (check_simulate_opt(&argc, argv))
    {
        flags |= OE_ENCLAVE_FLAG_SIMULATE;
    }

    if (argc != 2)
    {
        fprintf(
            stderr, "Usage: %s enclave_image_path [ --simulate  ]\n", argv[0]);
        goto exit;
    }

    // Initialize log
    const char* log_path = "mylog.txt";
    ret = log_init(LOG_DEBUG, log_path);
    if (ret != 0)
    {
        fprintf(stderr, "log_init(): error=%u\n", ret);
        goto exit;
    }

    // Create the enclave
    result = oe_create_logs_enclave(
        argv[1], OE_ENCLAVE_TYPE_SGX, flags, NULL, 0, &enclave);
    if (result != OE_OK)
    {
        fprintf(
            stderr,
            "oe_create_logs_enclave(): result=%u (%s)\n",
            result,
            oe_result_str(result));
        goto exit;
    }

    // Call into the enclave
    result = enclave_logs(enclave);
    if (result != OE_OK)
    {
        fprintf(
            stderr,
            "calling into enclave_logs failed: result=%u (%s)\n",
            result,
            oe_result_str(result));
        goto exit;
    }

    // Close log
    log_close();

    // Read the log
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(log_path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "fopen(%s): failed\n", log_path);
        goto exit;
    }
    printf("---LOG-BEGIN---\n");
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("%s", line);
    }
    printf("---LOG-END---\n");

    // Clean up
    fclose(fp);
    if (line)
        free(line);
    remove(log_path);

    ret = 0;

exit:
    // Clean up the enclave if we created one
    if (enclave)
        oe_terminate_enclave(enclave);

    return ret;
}
