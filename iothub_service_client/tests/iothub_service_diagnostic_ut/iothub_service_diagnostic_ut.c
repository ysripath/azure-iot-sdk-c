// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cstdint>
#else
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#endif

static char* TestDeviceTwin = "{\"properties\": {\"desired\": {\"__e2e_diag_sample_rate\": 90,\"$version\": 24}}}";
static char* TestDeviceTwinNoDiag = "{\"properties\": {\"desired\": {\"$version\": 24}}}";
static char* TestDeviceTwinInvalidDiag = "{\"properties\": {\"desired\": {\"__e2e_diag_sample_rate\": \"abc\",\"$version\": 24}}}";

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* ptr)
{
    if (ptr == NULL)
        return;

    char* str = (char*)ptr;
    if (strcmp(TestDeviceTwin, str) != 0 && strcmp(TestDeviceTwinNoDiag, str) != 0 && strcmp(TestDeviceTwinInvalidDiag, str) != 0)
    {
        free(ptr);
    }
}

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umock_c_negative_tests.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"
#include "umocktypes_stdint.h"
#include "parson.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/buffer_.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/lock.h"
#include "iothub_devicetwin.h"

#undef ENABLE_MOCKS

#include "iothub_service_diagnostic.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    (void)error_code;
    ASSERT_FAIL("umock_c reported error");
}

static IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE TEST_IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE = (IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE)0x1234;
static const char* TestDeviceId = "testDeviceId";

BEGIN_TEST_SUITE(iothub_service_diagnostic_ut)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    int result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_GLOBAL_MOCK_RETURN(IoTHubDeviceTwin_GetTwin, TestDeviceTwin);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubDeviceTwin_GetTwin, NULL);

    REGISTER_GLOBAL_MOCK_RETURN(IoTHubDeviceTwin_UpdateTwin, TestDeviceTwin);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(IoTHubDeviceTwin_UpdateTwin, NULL);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    umock_c_negative_tests_deinit();
    TEST_MUTEX_RELEASE(g_testByTest);
}

/*Codes_SRS_IOTHUBSERVICEDIAG_01_001: [ If failed to call IoTHubDeviceTwin_GetTwin, IoTHub_E2EDiagnostic_GetSetting shall return NULL]*/
TEST_FUNCTION(IoTHub_E2EDiagnostic_GetSetting_null_parameters_fail)
{
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = IoTHub_E2EDiagnostic_GetSetting(NULL, NULL);

    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Codes_SRS_IOTHUBSERVICEDIAG_01_002: [ If failed to call IoTHubDeviceTwin_GetTwin, IoTHub_E2EDiagnostic_GetSetting shall return NULL]*/
TEST_FUNCTION(IoTHub_E2EDiagnostic_GetSetting_fail_get_devicetwin)
{
    // arrange
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(IoTHubDeviceTwin_GetTwin(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).IgnoreAllArguments().SetReturn(NULL);

    // act
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = IoTHub_E2EDiagnostic_GetSetting(TEST_IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, TestDeviceId);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Codes_SRS_IOTHUBSERVICEDIAG_01_003: [ If no diagnostic setting in device twin, IoTHub_E2EDiagnostic_GetSetting shall return NULL]*/
TEST_FUNCTION(IoTHub_E2EDiagnostic_GetSetting_no_diagnostic_setting_in_devicetwin)
{
    // arrange
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(IoTHubDeviceTwin_GetTwin(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).IgnoreAllArguments().SetReturn(TestDeviceTwinNoDiag);

    // act
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = IoTHub_E2EDiagnostic_GetSetting(TEST_IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, TestDeviceId);

    // assert
    ASSERT_IS_NULL(result);
}

/*Codes_SRS_IOTHUBSERVICEDIAG_01_004: [ If diagnostic setting is invalid in device twin, IoTHub_E2EDiagnostic_GetSetting shall return NULL]*/
TEST_FUNCTION(IoTHub_E2EDiagnostic_GetSetting_invalid_diagnostic_setting_in_devicetwin)
{
    // arrange
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(IoTHubDeviceTwin_GetTwin(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(TestDeviceTwinInvalidDiag);

    // act
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = IoTHub_E2EDiagnostic_GetSetting(TEST_IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, TestDeviceId);

    // assert
    ASSERT_IS_NULL(result);
}

/*Codes_SRS_IOTHUBSERVICEDIAG_01_005: [ IoTHub_E2EDiagnostic_GetSetting shall return diagnostic setting upon success]*/
TEST_FUNCTION(IoTHub_E2EDiagnostic_GetSetting_success)
{
    // arrange
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(IoTHubDeviceTwin_GetTwin(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // act
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = IoTHub_E2EDiagnostic_GetSetting(TEST_IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, TestDeviceId);

    // assert
    ASSERT_IS_NOT_NULL(result);
}

/*Codes_SRS_IOTHUBSERVICEDIAG_01_006: [IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if any parameter is NULL]*/
TEST_FUNCTION(IoTHub_E2EDiagnostic_UpdateSetting_null_parameters_fail)
{
    // arrange
    umock_c_reset_all_calls();

    // act
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = IoTHub_E2EDiagnostic_UpdateSetting(NULL, NULL, NULL);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Codes_SRS_IOTHUBSERVICEDIAG_01_007: [IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if diagnostic sampling rating is not [0, 100]]*/
TEST_FUNCTION(IoTHub_E2EDiagnostic_UpdateSetting_sampling_rate_out_of_range)
{
    // arrange
    umock_c_reset_all_calls();

    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING diagSetting;
    diagSetting.diagSamplingPercentage = 101;

    // act
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = IoTHub_E2EDiagnostic_UpdateSetting(TEST_IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, TestDeviceId, &diagSetting);

    // assert
    ASSERT_IS_NULL(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/*Codes_SRS_IOTHUBSERVICEDIAG_01_008: [IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if failed to malloc]*/
/*Codes_SRS_IOTHUBSERVICEDIAG_01_009: [IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if failed to call IoTHubDeviceTwin_UpdateTwin]*/
TEST_FUNCTION(IoTHub_E2EDiagnostic_UpdateSetting_negative_cases)
{
    // arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    umock_c_reset_all_calls();

    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)).IgnoreAllArguments();
    EXPECTED_CALL(IoTHubDeviceTwin_UpdateTwin(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING diagSetting;
    diagSetting.diagSamplingPercentage = 100;

    umock_c_negative_tests_snapshot();

    // act
    size_t count = umock_c_negative_tests_call_count();

    for (size_t index = 0; index < count; index++)
    {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = IoTHub_E2EDiagnostic_UpdateSetting(TEST_IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, TestDeviceId, &diagSetting);
        
        ASSERT_IS_NULL(result);
    }

    umock_c_negative_tests_deinit();
}

/*Codes_SRS_IOTHUBSERVICEDIAG_01_010: [IoTHub_E2EDiagnostic_UpdateSetting shall return diagnostic setting upon success]*/
TEST_FUNCTION(IoTHub_E2EDiagnostic_UpdateSetting_success)
{
    // arrange
    umock_c_reset_all_calls();

    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING diagSetting;
    diagSetting.diagSamplingPercentage = 100;

    // act
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = IoTHub_E2EDiagnostic_UpdateSetting(TEST_IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, TestDeviceId, &diagSetting);

    // assert
    ASSERT_IS_NOT_NULL(result);
}

END_TEST_SUITE(iothub_service_diagnostic_ut)
