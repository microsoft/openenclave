Attestation QPL Enforcement Support
====

This proposal is to make a change to `oe_verify_evidence`, so that it can support policy based QPL (Quote Provider Library) enhancement, such as allowing the API user to pass data to QPL for more advanced quote verification.

Motivation
----

Intel SGX rolls out PCK certificate and collateral information about every six month. There is a scenario that a quote is signed by the last version of PCK certificate cannot be verified with the latest version of collateral information. This causes unnecessary attestation failures when new versions of PCK certificate and collateral information are rolled out. To deal with this issue, the QPL need to be able to return any version collateral information based on additional query parameters (referred as baselines). With that, a new API called `sgx_ql_get_quote_verification_collateral_with_params` is introduced in QPL. Changes are also needed at OE SDK to call this new API.

Goals:
 - Verifier can call `oe_verify_evidence` and pass QPL specific parameters to do more advanced quote verification.

User Experience
----

A new policy type `OE_POLICY_COLLATERAL_BASELINE` is added to current policy types. Quote verifier can pass multiple policies with this type, while OE SDK would pass all the policies down to the QPL API without interpreting the data content.

Specification
----

### New policy type & its data structure

- OE_POLICY_COLLATERAL_BASELINE
- Data structure is defined as below,
```c
// defined by QPL
#define MAX_PARAM_STRING_SIZE (256)
typedef struct _sgx_ql_qve_collateral_t
{
uint8_t key[MAX_PARAM_STRING_SIZE+1];
uint8_t value[MAX_PARAM_STRING_SIZE+1];
} sgx_ql_qve_collateral_t;
```

### New APIs / OCalls

Though there is no API signature changes for `oe_verify_evidence`, some internal APIs that potentially get used by tools need to be changed. For backward compatibility, the following new APIs wil be added to ensure all existing code either build from OE SDK headers and libraries or source directly can still work,
- `oe_get_sgx_endorsements_with_policies`, after change, this API will be called by existing `oe_get_sgx_endorsements` without any policy.
- `oe_get_quote_verification_collateral_with_params_ocall`, the current API is `oe_get_quote_verification_collateral_ocall` is an API defined in sgx/attestation.edl, which could have been used by user enclaves, so need to bring this new API to work with existing API.

Alternatives
----

Since there could be other parameters required by QPL in future, the new policy type can be named to `OE_POLICY_QPL_PARAMETER`, which makes the policy more generic and the verifier can call `oe_verify_evidence` with arbitrary number of parameters with data format confronts to
```c
typedef struct _oe_qpl_parameter {
    uint32_t tag;
    uint8_t data[0];
} oe_qpl_parameter;
```

With this, OE SDK will blindly pass all the parameters to all QPL APIs and QPL APIs need to use the `tag` for each parameter to figure out the parameters that they can take and also the format for the data in parameter. However, this needs tag and data protocol for parameters between the quote verifier and QPL vendors, which will introduce other difficulties for OE SDK users. For example, in case of multiple QPL vendors, for the same API in QPL, they take different formats of parameter data but same tag, now the `oe_verify_evidence` caller has to know which QPL vendor is going to be used, which removes the abstractions that OE SDK provides.