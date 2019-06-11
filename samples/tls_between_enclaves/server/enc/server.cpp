// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include <mbedtls/certs.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/debug.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/pk.h>
#include <mbedtls/platform.h>
#include <mbedtls/rsa.h>
#include <mbedtls/ssl.h>
#include <mbedtls/ssl_cache.h>
#include <mbedtls/x509.h>
#include <openenclave/enclave.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "../../common/utility.h"

extern "C"
{
    int setup_tls_server_and_run(char* server_port);
};

// mbedtls debug levels
// 0 No debug, 1 Error, 2 State change, 3 Informational, 4 Verbose
#define DEBUG_LEVEL 1
#define SERVER_IP "0.0.0.0"
#define MAX_ERROR_BUFF_SIZE 256
char error_buf[MAX_ERROR_BUFF_SIZE];
unsigned char buf[1024];

static void your_debug(
    void* ctx,
    int level,
    const char* file,
    int line,
    const char* str)
{
    ((void)level);

    mbedtls_fprintf((FILE*)ctx, "%s:%04d: %s", file, line, str);
    fflush((FILE*)ctx);
}

int configure_server_ssl(
    mbedtls_ssl_context* ssl,
    mbedtls_ssl_config* conf,
    mbedtls_ssl_cache_context* cache,
    mbedtls_ctr_drbg_context* ctr_drbg,
    mbedtls_x509_crt* server_cert,
    mbedtls_pk_context* pkey)
{
    int ret = 1;
    oe_result_t result = OE_FAILURE;

    printf("Generating the certificate\n");
    result = generate_tls_certificate(server_cert, pkey);
    if (result != OE_OK)
    {
        printf("failed with %s\n", oe_result_str(result));
        ret = 1;
        goto exit;
    }

    printf("\nSetting up the TLS configuration....\n");
    if ((ret = mbedtls_ssl_config_defaults(
             conf,
             MBEDTLS_SSL_IS_SERVER,
             MBEDTLS_SSL_TRANSPORT_STREAM,
             MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        printf(
            "failed\n  ! mbedtls_ssl_config_defaults returned failed %d\n",
            ret);
        goto exit;
    }

    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);
    mbedtls_ssl_conf_dbg(conf, your_debug, stdout);
    mbedtls_ssl_conf_session_cache(
        conf, cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);

    // need to set authmode mode to OPTIONAL for requesting client certificate
    mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_verify(conf, cert_verify_callback, NULL);
    mbedtls_ssl_conf_ca_chain(conf, server_cert->next, NULL);

    if ((ret = mbedtls_ssl_conf_own_cert(conf, server_cert, pkey)) != 0)
    {
        printf("failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n", ret);
        goto exit;
    }

    if ((ret = mbedtls_ssl_setup(ssl, conf)) != 0)
    {
        printf("failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
        goto exit;
    }
    ret = 0;
exit:
    fflush(stdout);
    return ret;
}

// This routine was created to demonstrate a simple communication scenario
// between a TLS client and an TLS server. In a real TLS server app, you
// definitely will have to do more that just receiving a single message
// from a client.
int handle_communication_until_done(
    mbedtls_ssl_context* ssl,
    mbedtls_net_context* listen_fd,
    mbedtls_net_context* client_fd)
{
    int ret = 0;
    int len = 0;

waiting_for_connection_request:

    if (ret != 0)
    {
        mbedtls_strerror(ret, error_buf, MAX_ERROR_BUFF_SIZE);
        printf("Last error was: %d - %s\n", ret, error_buf);
    }

    // reset ssl setup and client_fd to prepare for the new TLS connection
    mbedtls_net_free(client_fd);
    mbedtls_ssl_session_reset(ssl);

    printf("Waiting for a client connection request...\n");
    if ((ret = mbedtls_net_accept(listen_fd, client_fd, NULL, 0, NULL)) != 0)
    {
        char errbuf[512];
        mbedtls_strerror(ret, errbuf, sizeof(errbuf));
        printf(" failed\n  ! mbedtls_net_accept returned %d\n\n", ret);
        printf("%s\n", errbuf);
        goto done;
    }
    printf(
        "mbedtls_net_accept returned successfully.(listen_fd = %d) (client_fd "
        "= %d) \n",
        listen_fd->fd,
        client_fd->fd);

    // set up bio callbacks
    mbedtls_ssl_set_bio(
        ssl, client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    printf("Performing the SSL/TLS handshake...\n");
    while ((ret = mbedtls_ssl_handshake(ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n", -ret);
            goto done;
        }
    }

    printf("mbedtls_ssl_handshake done successfully\n");

    // read client's request
    printf("<---- Read from client:\n");
    do
    {
        len = sizeof(buf) - 1;
        memset(buf, 0, sizeof(buf));
        ret = mbedtls_ssl_read(ssl, buf, len);

        if (ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE)
            continue;

        if (ret <= 0)
        {
            switch (ret)
            {
                case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                    printf("connection was closed gracefully\n");
                    break;

                case MBEDTLS_ERR_NET_CONN_RESET:
                    printf("connection was reset by peer\n");
                    break;

                default:
                    printf("mbedtls_ssl_read returned -0x%x\n", -ret);
                    break;
            }
            break;
        }

        len = ret;
        printf(" %d bytes received from client:\n[%s]\n", len, (char*)buf);

#ifdef ADD_TEST_CHECKING
        if ((len != CLIENT_PAYLOAD_SIZE) ||
            (memcmp(PAYLOAD_FROM_CLIENT, buf, len) != 0))
        {
            printf(
                "ERROR: expected reading %d bytes but only got %d bytes\n",
                (int)CLIENT_PAYLOAD_SIZE,
                len);
            ret = MBEDTLS_EXIT_FAILURE;
            goto done;
        }
        printf("Verified: the contents of client payload were expected\n\n");
#endif
        if (ret > 0)
            break;
    } while (1);

    // Write a response back to the client
    printf("-----> Write to client:\n");
    len = snprintf((char*)buf, sizeof(buf) - 1, PAYLOAD_FROM_SERVER);

    while ((ret = mbedtls_ssl_write(ssl, buf, len)) <= 0)
    {
        if (ret == MBEDTLS_ERR_NET_CONN_RESET)
        {
            printf(" failed\n  ! peer closed the connection\n\n");
            goto waiting_for_connection_request;
        }
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
            goto done;
        }
    }

    len = ret;
    printf(" %d bytes written to client\n\n", len);
    printf("Closing the connection...\n");
    while ((ret = mbedtls_ssl_close_notify(ssl)) < 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            printf("failed! mbedtls_ssl_close_notify returned %d\n\n", ret);
            goto waiting_for_connection_request;
        }
    }

    ret = 0;
    // comment out the following line if you want the server in a loop
    // goto waiting_for_connection_request;

done:
    return ret;
}

int setup_tls_server_and_run(char* server_port)
{
    int ret = 0;
    oe_result_t result = OE_FAILURE;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt server_cert;
    mbedtls_pk_context pkey;
    mbedtls_ssl_cache_context cache;
    mbedtls_net_context listen_fd, client_fd;
    const char* pers = "tls_server";

    // init mbedtls objects
    mbedtls_net_init(&listen_fd);
    mbedtls_net_init(&client_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ssl_cache_init(&cache);
    mbedtls_x509_crt_init(&server_cert);
    mbedtls_pk_init(&pkey);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_debug_set_threshold(DEBUG_LEVEL);

    // explicitly load socket and host resolver features
    if ((result = oe_load_module_host_resolver()) != OE_OK)
    {
        printf(
            "oe_load_module_host_resolver failed with %s\n",
            oe_result_str(result));
        goto exit;
    }
    if ((result = oe_load_module_host_socket_interface()) != OE_OK)
    {
        printf(
            "oe_load_module_host_socket_interface failed with %s\n",
            oe_result_str(result));
        goto exit;
    }

    // set up server port
    printf(
        "Setup the listening TCP socket on SERVER_IP= [%s] server_port = "
        "[%s]\n",
        SERVER_IP,
        server_port);
    if ((ret = mbedtls_net_bind(
             &listen_fd, SERVER_IP, server_port, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        printf(" failed\n  ! mbedtls_net_bind returned %d\n", ret);
        goto exit;
    }

    printf("mbedtls_net_bind succeeded (listen_fd = %d)\n", listen_fd.fd);

    if ((ret = mbedtls_ctr_drbg_seed(
             &ctr_drbg,
             mbedtls_entropy_func,
             &entropy,
             (const unsigned char*)pers,
             strlen(pers))) != 0)
    {
        printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        goto exit;
    }

    // configure server tls settings
    ret = configure_server_ssl(
        &ssl, &conf, &cache, &ctr_drbg, &server_cert, &pkey);
    if (ret != 0)
    {
        printf(" failed\n  ! mbedtls_net_connect returned %d\n", ret);
        goto exit;
    }

    // handle communicate and server communication
    ret = handle_communication_until_done(&ssl, &listen_fd, &client_fd);
    if (ret != 0)
    {
        printf("server communication error %d\n", ret);
        goto exit;
    }

exit:

    if (ret != 0)
    {
        mbedtls_strerror(ret, error_buf, MAX_ERROR_BUFF_SIZE);
        printf("Last error was: %d - %s\n\n", ret, error_buf);
    }

    // free resource
    mbedtls_net_free(&client_fd);
    mbedtls_net_free(&listen_fd);
    mbedtls_x509_crt_free(&server_cert);
    mbedtls_pk_free(&pkey);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ssl_cache_free(&cache);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return (ret);
}
