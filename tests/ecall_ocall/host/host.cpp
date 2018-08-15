// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#define OE_TRACE_LEVEL 1

#include <openenclave/host.h>
#include <openenclave/internal/error.h>
#include <openenclave/internal/tests.h>
#include <openenclave/internal/trace.h>
#include <openenclave/internal/types.h>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <system_error>
#include <thread>
#include <vector>
#include "../args.h"
#include "../crc32.h"

#define THREAD_COUNT 5 // must not exceed what is configured in sign.conf

// Slightly specialized wrapper around an oe_enclave_t object to allow
// scope-based lifetime mgmt. Also a bit of identifying glue (which relies on
// custom code in the enclave).
struct EnclaveWrap
{
    EnclaveWrap(const char* enclavePath, uint32_t flags)
    {
        EncSetEnclaveIdArg args = {};
        oe_enclave_t* enclave;
        oe_result_t result;

        if ((result = oe_create_enclave(
                 enclavePath, OE_ENCLAVE_TYPE_SGX, flags, NULL, 0, &enclave)) !=
            OE_OK)
        {
            oe_put_err("oe_create_enclave(): result=%u", result);
            throw std::runtime_error("oe_create_enclave() failed");
        }
        m_Id = m_Enclaves.size();

        args.result = OE_FAILURE;
        args.id = m_Id;
        if ((result = oe_call_enclave(enclave, "EncSetEnclaveId", &args)) !=
            OE_OK)
        {
            oe_put_err("oe_call_enclave(EncSetEnclaveId): result=%u", result);
            throw std::runtime_error("oe_call_enclave(EncSetEnclaveId) failed");
        }
        if (args.result != OE_OK)
        {
            oe_put_err("EncSetEnclaveId(): result=%u", result);
            throw std::runtime_error("EncSetEnclaveId() failed");
        }

        m_EnclaveBase = args.baseAddr;
        m_Enclaves.push_back(enclave);
    }

    ~EnclaveWrap()
    {
        oe_result_t result;
        if ((result = oe_terminate_enclave(Get())) != OE_OK)
        {
            oe_put_err("oe_terminate_enclave(): result=%u", result);
        }
        // simplified cleanup to keep identifiers stable
        m_Enclaves[m_Id] = NULL;
    }

    unsigned GetId() const
    {
        return m_Id;
    }
    const void* GetBase() const
    {
        return m_EnclaveBase;
    }
    oe_enclave_t* Get() const
    {
        return m_Enclaves[m_Id];
    }

    static oe_enclave_t* Get(unsigned Id)
    {
        return m_Enclaves[Id];
    }

  private:
    unsigned m_Id;
    const void* m_EnclaveBase;
    static std::vector<oe_enclave_t*> m_Enclaves;
};
std::vector<oe_enclave_t*> EnclaveWrap::m_Enclaves;

static std::vector<void*> InitOCallValues;

// OCall handler for initial ocall testing - track argument for later
// verification
OE_OCALL void InitOcallHandler(void* arg_)
{
    InitOCallValues.push_back(arg_);
}

// Initial OCall test helper - Verify that the ocall happened (by asking the
// enclave), and obtain the result of it.
static void TestInitOcallResult(unsigned enclaveId)
{
    oe_result_t result, resultOcall;

    resultOcall = OE_FAILURE;
    result = oe_call_enclave(
        EnclaveWrap::Get(enclaveId), "EncGetInitOcallResult", &resultOcall);
    OE_TEST(result == OE_OK);
    OE_TEST(resultOcall == OE_OK);
}

// For ocall-test on not explicitly OE_OCALL-tagged function
#if defined(__unix__)
extern "C" void ExportedHostFunction(void*)
#elif defined(_WIN32)
extern "C" OE_EXPORT void ExportedHostFunction(void*)
#endif
{
}

#if defined(_WIN32)
extern "C" void InternalHostFunction(void*)
{
}
#else
extern "C" void __attribute__((visibility("internal")))
InternalHostFunction(void*)
{
}
#endif

// Test availability and non-availability of functions, according to their
// OE_OCALL/OE_ECALL annotations.
static void TestInvalidFunctions(unsigned enclaveId)
{
    oe_result_t result;
    EncTestCallHostFunctionArg args = {};

    result = oe_call_enclave(
        EnclaveWrap::Get(enclaveId), "EncDummyEncFunction", NULL);
    printf("oe_call_enclave(EncDummyEncFunction): %u\n", result);
    OE_TEST(result == OE_OK);

    result = oe_call_enclave(
        EnclaveWrap::Get(enclaveId), "EncUnExportedFunction", NULL);
    printf("oe_call_enclave(EncUnExportedFunction): %u\n", result);
    OE_TEST(result == OE_NOT_FOUND);

    result = oe_call_enclave(
        EnclaveWrap::Get(enclaveId), "NonExistingFunction", NULL);
    printf("oe_call_enclave(NonExistingFunction): %u\n", result);
    OE_TEST(result == OE_NOT_FOUND);

    args.result = OE_FAILURE;
    args.functionName = "InternalHostFunction";
    result = oe_call_enclave(
        EnclaveWrap::Get(enclaveId), "EncTestCallHostFunction", &args);
    printf(
        "oe_call_enclave(EncTestCallHostFunction, InternalHostFunction): "
        "%u/%u\n",
        result,
        args.result);
    OE_TEST(result == OE_OK);
    OE_TEST(args.result == OE_NOT_FOUND);

    args.result = OE_FAILURE;
    args.functionName = "NonExistingFunction";
    result = oe_call_enclave(
        EnclaveWrap::Get(enclaveId), "EncTestCallHostFunction", &args);
    printf(
        "oe_call_enclave(EncTestCallHostFunction, NonExistingFunction): "
        "%u/%u\n",
        result,
        args.result);
    OE_TEST(result == OE_OK);
    OE_TEST(args.result == OE_NOT_FOUND);

    args.result = OE_FAILURE;
    args.functionName = "ExportedHostFunction";
    result = oe_call_enclave(
        EnclaveWrap::Get(enclaveId), "EncTestCallHostFunction", &args);
    printf(
        "oe_call_enclave(EncTestCallHostFunction, ExportedHostFunction): "
        "%u/%u\n",
        result,
        args.result);
    OE_TEST(result == OE_OK);
    OE_TEST(args.result == OE_OK);
}

// Helper function for parallel test
static void ParallelThread(
    unsigned enclaveId,
    unsigned flowId,
    volatile unsigned* counter,
    volatile unsigned* release)
{
    oe_result_t result;

    EncParallelExecutionArg args = {};
    args.result = OE_FAILURE;
    args.enclaveId = enclaveId;
    args.flowId = flowId;
    args.counter = counter;
    args.release = release;

    OE_TRACE_INFO(
        "%s(Enclave=%u, Flow=%u) started\n", __FUNCTION__, enclaveId, flowId);
    result = oe_call_enclave(
        EnclaveWrap::Get(enclaveId), "EncParallelExecution", &args);
    OE_TRACE_INFO(
        "%s(Enclave=%u, Flow=%u) done.\n", __FUNCTION__, enclaveId, flowId);
    OE_TEST(result == OE_OK);
    OE_TEST(args.result == OE_OK);
}

// Parallel execution test - verify parallel threads are actually executed
static void TestExecutionParallel(
    std::vector<unsigned> enclaveIds,
    unsigned threadCount)
{
    std::vector<std::thread> threads;
    volatile unsigned counter = 0;
    volatile unsigned release = 0;

    printf("%s(): Test parallel execution across enclaves {", __FUNCTION__);
    for (unsigned e : enclaveIds)
        printf("%u ", e);
    printf("} with %u threads each\n", threadCount);

    counter = release = 0;

    for (unsigned enclaveId : enclaveIds)
    {
        for (unsigned i = 0; i < threadCount; i++)
        {
            threads.push_back(
                std::thread(
                    ParallelThread, enclaveId, i + 1, &counter, &release));
        }
    }

    // wait for all enclave-threads to have incremented the counter
    while (counter < enclaveIds.size() * threadCount)
    {
#if (OE_TRACE_LEVEL >= OE_TRACE_LEVEL_INFO)

        static unsigned oldVal;
        if (counter != oldVal)
        {
            printf(
                "%s(): Looking for counter=%u, have %u.\n",
                __FUNCTION__,
                (unsigned)EnclaveIds.size() * threadCount,
                counter);
            oldVal = counter;
        }
#endif
    }

    // all threads arrived and spin on the release
    release = 1;

    for (auto& t : threads)
    {
        t.join();
    }
}

static std::set<unsigned> g_rotatingEnclaveIds;

// Ocall for recursion test
OE_OCALL void RecursionOcall(void* args_)
{
    oe_result_t result = OE_OK;

    EncRecursionArg* argsPtr = (EncRecursionArg*)args_;
    EncRecursionArg args = *argsPtr;
    EncRecursionArg argsRec;

    OE_TRACE_INFO(
        "%s(): EnclaveId=%u, Flow=%u, recLeft=%u, inCrc=%#x\n",
        __FUNCTION__,
        args.enclaveId,
        args.flowId,
        args.recursionsLeft,
        args.crc);

    // catch initial state: Tag + Input-struct
    args.crc = Crc32::Hash(TAG_START_HOST, args);
    argsRec = args;

    // recurse as needed, passing initial-state-crc as input
    if (args.recursionsLeft)
    {
        argsRec.recursionsLeft--;
        if (argsRec.isRotatingEnclave)
        {
            std::set<unsigned>::iterator it =
                g_rotatingEnclaveIds.find(args.enclaveId);
            OE_TEST(it != g_rotatingEnclaveIds.end());
            if (++it == g_rotatingEnclaveIds.end())
            {
                it = g_rotatingEnclaveIds.begin();
            }
            argsRec.enclaveId = *it;
        }
        result = oe_call_enclave(
            EnclaveWrap::Get(argsRec.enclaveId), "EncRecursion", &argsRec);
    }

    // catch output state: Tag + result + output, and again original input
    argsPtr->crc = Crc32::Hash(TAG_END_HOST, result, argsRec, args);
}

static uint32_t CalcRecursionHashHost(const EncRecursionArg* args_);
static uint32_t CalcRecursionHashEnc(const EncRecursionArg* args_);

// calc recursion hash locally, host part
static uint32_t CalcRecursionHashHost(const EncRecursionArg* args_)
{
    EncRecursionArg args = *args_;
    EncRecursionArg argsRec;
    oe_result_t result = OE_OK;

    OE_TRACE_INFO(
        "%s(): EnclaveId=%u, Flow=%u, recLeft=%u, inCrc=%#x\n",
        __FUNCTION__,
        args.enclaveId,
        args.flowId,
        args.recursionsLeft,
        args.crc);

    // catch initial state: Tag + Input-struct
    args.crc = Crc32::Hash(TAG_START_HOST, args);
    argsRec = args;

    // recurse as needed, passing initial-state-crc as input
    if (args.recursionsLeft)
    {
        argsRec.recursionsLeft--;
        if (argsRec.isRotatingEnclave)
        {
            std::set<unsigned>::iterator it =
                g_rotatingEnclaveIds.find(args.enclaveId);
            OE_TEST(it != g_rotatingEnclaveIds.end());
            if (++it == g_rotatingEnclaveIds.end())
            {
                it = g_rotatingEnclaveIds.begin();
            }
            argsRec.enclaveId = *it;
        }

        argsRec.crc = CalcRecursionHashEnc(&argsRec);
        if (argsRec.recursionsLeft)
        {
            if (argsRec.initialCount)
                argsRec.initialCount--;
            argsRec.recursionsLeft--;
        }
    }

    // catch output state: Tag + result + output, and again original input
    return Crc32::Hash(TAG_END_HOST, result, argsRec, args);
}

// calc recursion hash locally, enc part
static uint32_t CalcRecursionHashEnc(const EncRecursionArg* args_)
{
    EncRecursionArg args = *args_;
    EncRecursionArg argsHost;
    oe_result_t result = OE_OK;

    OE_TRACE_INFO(
        "%s(): EnclaveId=%u, Flow=%u, recLeft=%u, inCrc=%#x\n",
        __FUNCTION__,
        args.enclaveId,
        args.flowId,
        args.recursionsLeft,
        args.crc);

    // catch initial state: Tag + Input-struct + enclaveId
    args.crc = Crc32::Hash(TAG_START_ENC, args, 0u, 0u);
    argsHost = args;

    if (args.recursionsLeft > 0)
    {
        if (argsHost.initialCount)
            argsHost.initialCount--;
        argsHost.recursionsLeft--;
        argsHost.crc = CalcRecursionHashHost(&argsHost);
    }

    // catch output state: Tag + result + modified host-struct, original
    // input, and ID-diffs
    return Crc32::Hash(TAG_END_ENC, result, argsHost, args, 0u, 0u);
}

// Actual enclave/host/... recursion test. Trail of execution is gathered via
// Crc, success determined via comparison with separate, non-enclave version.
static uint32_t TestRecursion(
    size_t enclaveId,
    unsigned flowId,
    unsigned recursionDepth)
{
    oe_result_t result;
    EncRecursionArg args = {};

    OE_TRACE_INFO(
        "%s(EnclaveId=%llu, FlowId=%u, Recursions=%u)\n",
        __FUNCTION__,
        OE_LLU(enclaveId),
        flowId,
        recursionDepth);

    args.enclaveId = enclaveId;
    args.flowId = flowId;
    args.recursionsLeft = recursionDepth;
    args.initialCount = 1;

    uint32_t crc = CalcRecursionHashEnc(&args);

    result =
        oe_call_enclave(EnclaveWrap::Get(enclaveId), "EncRecursion", &args);
    OE_TEST(result == OE_OK);

    printf(
        "%s(EnclaveId=%llu, FlowId=%u, RecursionDepth=%u): "
        "Expect CRC %#x, have CRC %#x, %s\n",
        __FUNCTION__,
        OE_LLU(enclaveId),
        flowId,
        recursionDepth,
        crc,
        args.crc,
        (crc == args.crc) ? "MATCH" : "MISMATCH");
    OE_TEST(crc == args.crc);
    return crc;
}

// Thread helper to perform recursion tests in parallel across multiple threads.
static void RecursionThread(
    unsigned enclaveId,
    bool rotateEnclaves,
    unsigned flowId,
    unsigned recursionDepth,
    unsigned loopCount,
    uint32_t expectedCrc)
{
    // a barrier would be nice here, though we have no support in gcc yet.
    for (unsigned l = 0; l < loopCount; l++)
    {
        oe_result_t result;
        EncRecursionArg args = {};

        args.enclaveId = enclaveId;
        args.flowId = flowId;
        args.recursionsLeft = recursionDepth;
        args.initialCount = rotateEnclaves ? g_rotatingEnclaveIds.size() : 1;
        args.isRotatingEnclave = !!rotateEnclaves;

        result =
            oe_call_enclave(EnclaveWrap::Get(enclaveId), "EncRecursion", &args);
        OE_TEST(result == OE_OK);
        OE_TEST(args.crc == expectedCrc);
    }
}

// Parallel recursion test.
static void TestRecursionParallel(
    std::vector<unsigned> enclaveIds,
    unsigned threadCount,
    unsigned recursionDepth,
    unsigned loopCount)
{
    std::vector<std::thread> threads;
    std::map<unsigned, std::vector<uint32_t>> expectedCrcs;

    printf("%s(): Test recursion w/ multiple enclaves {", __FUNCTION__);
    for (unsigned e : enclaveIds)
        printf("%u ", e);
    printf(
        "} with %u threads each, rec-depth %u, loop-count %u\n",
        threadCount,
        recursionDepth,
        loopCount);

    // Precalculate CRCs
    for (unsigned enclaveId : enclaveIds)
    {
        for (unsigned i = 0; i < threadCount; i++)
        {
            EncRecursionArg args = {};
            args.enclaveId = enclaveId;
            args.flowId = i + 1;
            args.recursionsLeft = recursionDepth + i;
            args.initialCount = 1;

            expectedCrcs[enclaveId].push_back(CalcRecursionHashEnc(&args));
        }
    }

    for (unsigned enclaveId : enclaveIds)
    {
        for (unsigned i = 0; i < threadCount; i++)
        {
            threads.push_back(
                std::thread(
                    RecursionThread,
                    enclaveId,
                    false,
                    i + 1,
                    recursionDepth + i,
                    loopCount,
                    expectedCrcs[enclaveId][i]));
        }
    }

    for (auto& t : threads)
    {
        t.join();
    }
}

// Parallel recursion test, across threads
static void TestRecursionCrossEnclave(
    std::vector<unsigned> enclaveIds,
    unsigned threadCount,
    unsigned recursionDepth,
    unsigned loopCount)
{
    std::vector<std::thread> threads;
    std::vector<unsigned> expectedCrcs;

    printf("%s(): Test recursion across enclaves {", __FUNCTION__);
    for (unsigned e : enclaveIds)
        printf("%u ", e);
    printf(
        "} with %u threads total, rec-depth %u, loop-count %u\n",
        threadCount,
        recursionDepth,
        loopCount);

    g_rotatingEnclaveIds.clear();
    for (unsigned enclaveId : enclaveIds)
        g_rotatingEnclaveIds.insert(enclaveId);

    // Precalculate CRCs
    for (unsigned i = 0; i < threadCount; i++)
    {
        EncRecursionArg args = {};
        args.enclaveId = enclaveIds[i % enclaveIds.size()];
        args.flowId = i + 1;
        args.recursionsLeft = recursionDepth + i;
        args.initialCount = enclaveIds.size();
        args.isRotatingEnclave = 1;

        expectedCrcs.push_back(CalcRecursionHashEnc(&args));
    }

    for (unsigned i = 0; i < threadCount; i++)
    {
        threads.push_back(
            std::thread(
                RecursionThread,
                enclaveIds[i % enclaveIds.size()],
                true,
                i + 1,
                recursionDepth + i,
                loopCount,
                expectedCrcs[i]));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    g_rotatingEnclaveIds.clear();
}

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s ENCLAVE\n", argv[0]);
        exit(1);
    }

    const uint32_t flags = oe_get_create_flags();

    OE_TEST(InitOCallValues.size() == 0);
    EnclaveWrap enc1(argv[1], flags);
    // verify initial OCall succeeded
    OE_TEST(InitOCallValues.size() == 1);
    OE_TEST(InitOCallValues[0] != NULL);
    OE_TEST(InitOCallValues[0] == enc1.GetBase());
    TestInitOcallResult(enc1.GetId());

    // invalid function tests
    TestInvalidFunctions(enc1.GetId());

    // verify threads execute in parallel
    TestExecutionParallel({enc1.GetId()}, THREAD_COUNT);

    // serial recursion tests.
    OE_TEST(TestRecursion(enc1.GetId(), 1, 33) == 0xb6b66b4d);
    OE_TEST(TestRecursion(enc1.GetId(), 2, 33) == 0x721ea484);
    OE_TEST(TestRecursion(enc1.GetId(), 3, 100) == 0x83fab7f6);

    // Test in a 2nd enclave
    EnclaveWrap enc2(argv[1], flags);
    // verify initial OCall succeeded
    OE_TEST(InitOCallValues.size() == 2);
    OE_TEST(InitOCallValues[1] != NULL);
    OE_TEST(InitOCallValues[1] != InitOCallValues[0]);
    OE_TEST(InitOCallValues[1] == enc2.GetBase());
    TestInitOcallResult(enc2.GetId());

    // verify threads execute in parallel across enclaves
    TestExecutionParallel({enc1.GetId(), enc2.GetId()}, THREAD_COUNT);

    // verify recursion in 2nd enclave by itself
    OE_TEST(TestRecursion(enc2.GetId(), 1, 33) == 0xf3d31c55);

    // Test parallel recursion in one enclave
    TestRecursionParallel({enc1.GetId()}, THREAD_COUNT, 100, 3000);

    // And parallel with multiple enclaves
    TestRecursionParallel(
        {enc1.GetId(), enc2.GetId()}, THREAD_COUNT, 20, 10000);

    // Parallel across enclaves. Leave one thread unused to stir things, add
    // yet another enclave.
    EnclaveWrap enc3(argv[1], flags);

    TestRecursionCrossEnclave(
        {enc1.GetId(), enc2.GetId(), enc3.GetId()}, THREAD_COUNT - 1, 20, 1000);

    return 0;
}
