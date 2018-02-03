/**
 * \file enclave.h
 *
 * This file defines the programming interface for developing enclaves.
 *
 */
#ifndef _OE_ENCLAVE_H
#define _OE_ENCLAVE_H

#include "defs.h"
#include "types.h"
#include "result.h"
#include "thread.h"
#include "bits/sha.h"

OE_EXTERNC_BEGIN

#ifndef OE_BUILD_ENCLAVE
# define OE_BUILD_ENCLAVE
#endif

#define OE_ECALL OE_EXTERNC OE_EXPORT __attribute__((section (".ecall")))

#define OE_REPORT_DATA_SIZE 64

/**
 * Perform a high-level enclave function call (OCALL).
 *
 * Call the host function whose name is given by the **func** parameter.
 * The host must define a corresponding function with the following
 * prototype.
 *
 *     OE_OCALL void (*)(void* args);
 *
 * The meaning of the **args** parameter is defined by the implementer of the
 * function and may be null.
 *
 * This function is implemented using the low-level OE_ECall() interface
 * where the function number is given by the **OE_FUNC_CALL_HOST** constant.
 *
 * Note that the return value of this function only indicates the success of
 * the call and not of the underlying function. The OCALL implementation must
 * define its own error reporting scheme based on **args**.
 *
 * @param func The name of the enclave function that will be called.
 * @param args The arguments to be passed to the enclave function.
 *
 * @returns This function return **OE_OK** on success.
 *
 */
OE_Result OE_CallHost(
    const char *func,
    void *args);

/**
 * Check whether the given buffer is strictly within the enclave.
 *
 * Check whether the buffer given by the **ptr** and **size** parameters is
 * strictly within the enclave's memory. If so, return true. If any
 * portion of the buffer lies outside the enclave's memory, return false.
 *
 * @param ptr The pointer pointer to buffer.
 * @param size The size of buffer
 *
 * @retval true The buffer is strictly within the enclave.
 * @retval false At least some part of the buffer is outside the enclave.
 *
 */
bool OE_IsWithinEnclave(
    const void* ptr,
    size_t size);

/**
 * Check whether the given buffer is strictly outside the enclave.
 *
 * Check whether the buffer given by the **ptr** and **size** parameters is
 * strictly outside the enclave's memory. If so, return true. If any
 * portion of the buffer lies within the enclave's memory, return false.
 *
 * @param ptr The pointer to buffer.
 * @param size The size of buffer.
 *
 * @retval true The buffer is strictly outside the enclave.
 * @retval false At least some part of the buffer is within the enclave.
 *
 */
bool OE_IsOutsideEnclave(
    const void* ptr,
    size_t size);

/**
 * Get a report for use in remote attestation.
 *
 * This function creates a report to be used in remote attestation. The
 * report shall contain the data given by the **reportData** parameter.
 * The following steps are performed:
 *
 * - Calls into the host to request that the AESM service initialize the
 *   quote. This step obtains **target information** for the enclave that
 *   will eventually sign the quote (the Intel(R) quote enclave).
 *
 * - Executes the ENCLU.EREPORT instruction to generate the report, passing
 *   it the **target information** and **report data**. This instruction fills
 *   in the **report** output parameter.
 *
 * The next step is to pass the newly created report to the host so it can
 * get a quote for this report from the Intel(R) quote enclave. See the
 * OE_GetQuote() host function for further details.
 *
 * If the *reportSize* parameter is too small, this function resets it to
 * the required size and returns OE_BUFFER_TOO_SMALL.
 *
 * **Caution:** This function is experimental and subject to change.
 *
 * @param reportData The report data that will be included in the report.
 * @param report The buffer where the report will be copied.
 * @param reportSize The size of the **report** buffer.
 *
 * @retval OE_OK The report was successfully created.
 * @retval OE_INVALID_PARAMETER At least one parameter is invalid.
 * @retval OE_BUFFER_TOO_SMALL The **report** buffer is too small.
 * @retval OE_OUT_OF_MEMORY Failed to allocate host heap memory.
 *
 */
OE_Result OE_GetReportForRemoteAttestation(
    const uint8_t reportData[OE_REPORT_DATA_SIZE],
    void *report,
    size_t* reportSize);

/**
 * Print formatted characters to the host's console.
 *
 * This function writes formatted characters to the host console. Is is based
 * on OE_Vsnprintf(), which has limited support for format types.
 *
 * @param fmt The limited printf style format.
 *
 * @returns The number of characters that were written.
 *
 */
OE_PRINTF_FORMAT(1, 2)
int OE_HostPrintf(const char* fmt, ...);

/**
 * Allocates space for parameters of the next call to host on the host's stack frame.
 *
 * This function allocates **size** bytes of space on the stack frame of the
 * host. The returned address will be a multiple of **alignment** (if
 * non-zero). The allocated space is freed automatically when the OCALL
 * returns. If the stack overflows, the behavior is undefined.
 *
 * @param size The number of bytes to allocate.
 * @param alignment The alignment requirement (see above).
 * @param isZeroInit Whether the allocated memory is zero-initialized.
 *
 * @returns Returns the address of the allocated space.
 *
 */
void *OE_HostAllocForCallHost(size_t size, size_t alignment, bool isZeroInit);

/**
 * Allocate bytes from the host's heap.
 *
 * This function allocates **size** bytes from the host's heap and returns the
 * address of the allocated memory. The implementation performs an OCALL to
 * the host, which calls malloc(). To free the memory, it must be passed to
 * OE_HostFree().
 *
 * @param size The number of bytes to be allocated.
 *
 * @returns The allocated memory or NULL if unable to allocate the memory.
 *
 */
void* OE_HostMalloc(size_t size);

/**
 * Reallocate bytes from the host's heap.
 *
 * This function changes the size of the memory block pointed to by **ptr**
 * on the host's heap to **size** bytes. The memory block may be moved to a 
 * new location, which is returned by this function. The implementation 
 * performs an OCALL to the host, which calls realloc(). To free the memory,
 * it must be passed to OE_HostFree().
 *
 * @param ptr The memory block to change the size of. If NULL, this method 
 * allocates **size** bytes as if OE_HostMalloc was invoked. If not NULL, 
 * it should be a pointer returned by a previous call to OE_HostCalloc, 
 * OE_HostMalloc or OE_HostRealloc.
 * @param size The number of bytes to be allocated. If 0, this method 
 * deallocates the memory at **ptr**. If the new size is larger, the value
 * of the memory in the new allocated range is indeterminate. 
 *
 * @returns The pointer to the reallocated memory or NULL if **ptr** was 
 * freed by setting **size** to 0. This method also returns NULL if it was 
 * unable to reallocate the memory, in which case the original **ptr** 
 * remains valid and its contents are unchanged. 
 *
 */
void* OE_HostRealloc(void* ptr, size_t size);

/**
 * Allocate zero-filled bytes from the host's heap.
 *
 * This function allocates **size** bytes from the host's heap and fills it
 * with zero character. It returns the address of the allocated memory. The
 * implementation performs an OCALL to the host, which calls calloc().
 * To free the memory, it must be passed to OE_HostFree().
 *
 * @param size The number of bytes to be allocated and zero-filled.
 *
 * @returns The allocated memory or NULL if unable to allocate the memory.
 *
 */
void* OE_HostCalloc(size_t nmemb, size_t size);

/**
 * Releases allocated memory.
 *
 * This function releases memory allocated with OE_HostMalloc() or
 * OE_HostCalloc() by performing an OCALL where the host calls free().
 *
 * @param ptr Pointer to memory to be released or null.
 *
 */
void OE_HostFree(void* ptr);

/**
 * Make a heap copy of a string.
 *
 * This function allocates memory on the host's heap, copies the **str**
 * parameter to that memory, and returns a pointer to the newly allocated
 * memory.
 *
 * @param str The string to be copied.
 *
 * @returns A pointer to the newly allocated string or NULL if unable to
 * allocate the storage.
 */
char* OE_HostStrdup(const char* str);

/**
 * Abort execution by causing and illegal instruction exception.
 *
 * This function aborts execution by executing the UD2 instruction.
 */
void OE_Abort(void);

/**
 * Called whenever an assertion fails.
 *
 * This internal function is called when the expression of the OE_Assert()
 * macro evaluates to zero. For example:
 *
 *     OE_Assert(x > y);
 *
 * If the expression evaluates to zero, this function is called with the
 * string representation of the expression as well as the file, the line, and
 * the function name where the macro was expanded.
 *
 * The __OE_AssertFail() function performs a host call to print a message
 * and then calls OE_Abort().
 *
 * @param expr The argument of the OE_Assert() macro.
 * @param file The name of the file where OE_Assert() was invoked.
 * @param file The line number where OE_Assert() was invoked.
 * @param line The name of the function that invoked OE_Assert().
 *
 */
void __OE_AssertFail(
    const char *expr,
    const char *file,
    int line,
    const char *func);

#define OE_Assert(EXPR) \
    do \
    { \
        if (!(EXPR)) \
            __OE_AssertFail(#EXPR, __FILE__, __LINE__, __FUNCTION__); \
    } \
    while (0)

OE_EXTERNC_END

#endif /* _OE_ENCLAVE_H */
