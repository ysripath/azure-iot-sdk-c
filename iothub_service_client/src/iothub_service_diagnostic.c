// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdint.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"

#include "parson.h"
#include "iothub_devicetwin.h"
#include "iothub_sc_version.h"
#include "iothub_service_diagnostic.h"

#define DEVICE_TWIN_SAMPLING_RATE_KEY "__e2e_diag_sample_rate"
#define DEVICE_TWIN_E2E_DIAG_SETTING_MAX_LEN 60

static IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* parse_e2e_diagnostic_setting(const char* deviceTwin)
{
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = NULL;

    JSON_Value* json = NULL;
    JSON_Object* jsonObject;
    JSON_Object* propertyObject;
    JSON_Object* desiredObject;
    JSON_Value* diagValue;
    JSON_Value_Type diagValueType;

    if (deviceTwin == NULL)
    {
        LogError("Input parameter could not be NULL");
    }
    else if ((json = json_parse_string(deviceTwin)) == NULL)
    {
        LogError("Failed to call json_parse_string");
    }
    else if ((jsonObject = json_value_get_object(json)) == NULL)
    {
        LogError("Failed to call json_value_get_object");
    }
    else if ((propertyObject = json_object_get_object(jsonObject, "properties")) == NULL)
    {
        LogError("Failed to call json_object_get_object for 'properties'");
    }
    else if ((desiredObject = json_object_get_object(propertyObject, "desired")) == NULL)
    {
        LogError("Failed to call json_object_get_object for 'desired'");
    }
    else if ((diagValue = json_object_get_value(desiredObject, DEVICE_TWIN_SAMPLING_RATE_KEY)) == NULL)
    {
        LogError("Failed to get value for key %s", DEVICE_TWIN_SAMPLING_RATE_KEY);
    }
    else if ((diagValueType = json_value_get_type(diagValue)) != JSONNumber)
    {
        LogError("The value type for key %s must be number.", DEVICE_TWIN_SAMPLING_RATE_KEY);
    }
    else if ((result = (IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING*)malloc(sizeof(IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING))) == NULL)
    {
        LogError("Failed to malloc for IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING");
    }
    else
    {
        result->diagSamplingPercentage = (int)json_object_get_number(desiredObject, DEVICE_TWIN_SAMPLING_RATE_KEY);
    }

    if (json != NULL)
    {
        json_value_free(json);
    }

    return result;
}

IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* IoTHub_E2EDiagnostic_GetSetting(IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE serviceClientDeviceTwinHandle, const char* deviceId)
{
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result = NULL;
    char* deviceTwin;

    if (serviceClientDeviceTwinHandle == NULL || deviceId == NULL)
    {
        /*Codes_SRS_IOTHUBSERVICEDIAG_01_001: [ If failed to call IoTHubDeviceTwin_GetTwin, IoTHub_E2EDiagnostic_GetSetting shall return NULL]*/
        LogError("Input parameter cannot be NULL");
    }
    else if((deviceTwin = IoTHubDeviceTwin_GetTwin(serviceClientDeviceTwinHandle, deviceId)) == NULL)
    {
        /*Codes_SRS_IOTHUBSERVICEDIAG_01_002: [ If failed to call IoTHubDeviceTwin_GetTwin, IoTHub_E2EDiagnostic_GetSetting shall return NULL]*/
        LogError("Failed to call IoTHubDeviceTwin_GetTwin");
    }
    else
    {
        /*Codes_SRS_IOTHUBSERVICEDIAG_01_003: [ If no diagnostic setting in device twin, IoTHub_E2EDiagnostic_GetSetting shall return NULL]*/
        /*Codes_SRS_IOTHUBSERVICEDIAG_01_004: [ If diagnostic setting is invalid in device twin, IoTHub_E2EDiagnostic_GetSetting shall return NULL]*/
        /*Codes_SRS_IOTHUBSERVICEDIAG_01_005: [ IoTHub_E2EDiagnostic_GetSetting shall return diagnostic setting upon success]*/
        result = parse_e2e_diagnostic_setting(deviceTwin);
        free(deviceTwin);
    }

    return result;
}

IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* IoTHub_E2EDiagnostic_UpdateSetting(IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE serviceClientDeviceTwinHandle, const char* deviceId, IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* diagSetting)
{
    IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* result;
    if (serviceClientDeviceTwinHandle == NULL || deviceId == NULL || diagSetting == NULL)
    {
        /*Codes_SRS_IOTHUBSERVICEDIAG_01_006: [IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if any parameter is NULL]*/
        LogError("Input parameter cannot be NULL");
        result = NULL;
    }
    else
    {
        if (diagSetting->diagSamplingPercentage > 100)
        {
            /*Codes_SRS_IOTHUBSERVICEDIAG_01_007: [IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if diagnostic sampling rating is not [0, 100]]*/
            LogError("The range of diagnostic sampling percentage must be between [0, 100]");
            result = NULL;
        }
        else
        {
            char* responseTwin;
            char* diagJsonBuffer = (char*)malloc(DEVICE_TWIN_E2E_DIAG_SETTING_MAX_LEN);
            if (diagJsonBuffer == NULL)
            {
                /*Codes_SRS_IOTHUBSERVICEDIAG_01_008: [IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if failed to malloc]*/
                LogError("Failed to malloc for diagJsonBuffer");
                result = NULL;
            }
            else if (sprintf(diagJsonBuffer, "{\"properties\":{\"desired\":{\"%s\":%u}}}", DEVICE_TWIN_SAMPLING_RATE_KEY, diagSetting->diagSamplingPercentage) < 0)
            {
                LogError("Failed to sprintf diagJsonBuffer");
                result = NULL;
            }
            else if ((responseTwin = IoTHubDeviceTwin_UpdateTwin(serviceClientDeviceTwinHandle, deviceId, diagJsonBuffer)) == NULL)
            {
                /*Codes_SRS_IOTHUBSERVICEDIAG_01_009: [IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if failed to call IoTHubDeviceTwin_UpdateTwin]*/
                LogError("Failed to update diagnstoc setting");
                result = NULL;
            }
            else
            {
                /*Codes_SRS_IOTHUBSERVICEDIAG_01_010: [IoTHub_E2EDiagnostic_UpdateSetting shall return diagnostic setting upon success]*/
                result = parse_e2e_diagnostic_setting(responseTwin);
                free(responseTwin);
            }
            free(diagJsonBuffer);
        }
    }

    return result;
}
