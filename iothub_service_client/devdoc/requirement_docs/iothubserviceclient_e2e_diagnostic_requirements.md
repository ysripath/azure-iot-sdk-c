# IoTHub Service Client E2E Diagnostic Requirements

## Overview

IoTHub Service Client E2E Diagnostic allows retrieve and update diagnostic setting.

## Exposed API

```c
typedef struct IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING_TAG
{
    uint32_t diagSamplingPercentage;
} IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING;

extern IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* IoTHub_E2EDiagnostic_GetSetting(IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE serviceClientDeviceTwinHandle, const char* deviceId);

extern IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* IoTHub_E2EDiagnostic_UpdateSetting(IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE serviceClientDeviceTwinHandle, const char* deviceId, IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* diagSetting);
```

## IoTHub_E2EDiagnostic_GetSetting
```c
extern IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* IoTHub_E2EDiagnostic_GetSetting(IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE serviceClientDeviceTwinHandle, const char* deviceId);
```

**SRS_IOTHUBSERVICEDIAG_01_001: [**If failed to call IoTHubDeviceTwin_GetTwin, IoTHub_E2EDiagnostic_GetSetting shall return NULL**]**

**SRS_IOTHUBSERVICEDIAG_01_002: [**If failed to call IoTHubDeviceTwin_GetTwin, IoTHub_E2EDiagnostic_GetSetting shall return NULL**]**

**SRS_IOTHUBSERVICEDIAG_01_003: [**If no diagnostic setting in device twin, IoTHub_E2EDiagnostic_GetSetting shall return NULL**]**

**SRS_IOTHUBSERVICEDIAG_01_004: [**If diagnostic setting is invalid in device twin, IoTHub_E2EDiagnostic_GetSetting shall return NULL**]**

**SRS_IOTHUBSERVICEDIAG_01_005: [**IoTHub_E2EDiagnostic_GetSetting shall return diagnostic setting upon success**]**

## IoTHub_E2EDiagnostic_UpdateSetting
```c
extern IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* IoTHub_E2EDiagnostic_UpdateSetting(IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE serviceClientDeviceTwinHandle, const char* deviceId, IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING* diagSetting);
```
**SRS_IOTHUBSERVICEDIAG_01_006: [**IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if any parameter is NULL**]**

**SRS_IOTHUBSERVICEDIAG_01_007: [**IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if diagnostic sampling rating is not [0, 100**]**

**SRS_IOTHUBSERVICEDIAG_01_008: [**IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if failed to malloc**]**

**SRS_IOTHUBSERVICEDIAG_01_009: [**IoTHub_E2EDiagnostic_UpdateSetting shall return NULL if failed to call IoTHubDeviceTwin_UpdateTwin**]**

**SRS_IOTHUBSERVICEDIAG_01_010: [**IoTHub_E2EDiagnostic_UpdateSetting shall return diagnostic setting upon success**]**



