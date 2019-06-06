Unittests Transport Layer Security end-2-end scenarios
=====================

Test scenarios:

In this test, we have three test hooks for enabling test scenarios.

- fail_cert_verify_callback
   When set to true, it will force a tls connecting party to fail cert_verify_callback unconditionally.
   When set to false, cert_verify_callback will perform normally.

- fail_enclave_identity_verifier_callback;
   When set to true, it will force a tls connecting party to fail fail_enclave_identity_verifier_callback unconditionally.
   When set to false, enclave_identity_verifier_callback will perform normally.

- fail_oe_verify_attestation_certificate;
   When set to true, it simulates a failure return from calling oe_verify_attestation_certificate
   When set to false, fail_oe_verify_attestation_certificate will perform normally.

Here the scenarios covered in these tests via running run_scenarios_tests()

1. A positive scenario
    This is the case we hope always happen when both TLS connecting parties successfully validate each other's certificate and enclave identity.
    And everything works correctly all test hooks mentioned above set to false.

2. Negative scenarios

    We run tls connection tests three times, each with a tls connecting party set to true one of the following hook configurations while the other connecting party with all hooks set to false.

    fail_cert_verify_callback
    fail_enclave_identity_verifier_callback
    fail_oe_verify_attestation_certificate

    And we ran above three scenarios for both server and client roles. That got us six negative test scenarios.
