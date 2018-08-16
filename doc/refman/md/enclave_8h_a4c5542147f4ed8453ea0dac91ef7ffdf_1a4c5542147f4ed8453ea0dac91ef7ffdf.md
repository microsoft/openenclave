[Index](index.md)

---
# oe_get_seal_key_by_policy()

Get a symmetric encryption key derived from the specified policy and coupled to the enclave platform.

## Syntax

    oe_result_t oe_get_seal_key_by_policy(oe_seal_policy_t sealPolicy, uint8_t *keyBuffer, uint32_t *keyBufferSize, uint8_t *keyInfo, uint32_t *keyInfoSize)
## Description 



## Parameters

#### sealPolicy

The policy for the identity properties used to derive the seal key.

#### keyBuffer

The buffer to write the resulting seal key to.

#### keyBufferSize

The size of the **keyBuffer** buffer. If this is too small, this function sets it to the required size and returns OE_BUFFER_TOO_SMALL. When this function success, the number of bytes written to keyBuffer is set to it.

#### keyInfo

Optional buffer for the enclave-specific key information which can be used to retrieve the same key later, on a newer security version.

#### keyInfoSize

The size of the **keyInfo** buffer. If this is too small, this function sets it to the required size and returns OE_BUFFER_TOO_SMALL. When this function success, the number of bytes written to keyInfo is set to it.

## Return value

#### OE_OK

The seal key was successfully requested.

#### OE_INVALID_PARAMETER

At least one parameter is invalid.

#### OE_BUFFER_TOO_SMALL

The **keyBuffer** or **keyInfo** buffer is too small.

#### OE_UNEXPECTED

An unexpected error happened.

---
[Index](index.md)

