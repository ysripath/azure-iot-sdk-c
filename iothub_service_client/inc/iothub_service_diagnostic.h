// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file is under development and it is subject to change

#ifndef IOTHUB_SERVICE_DIAGNOSTIC_H
#define IOTHUB_SERVICE_DIAGNOSTIC_H

#ifdef __cplusplus
extern "C"
{
#else
#endif

#include <stdint.h>
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/map.h"
#include "azure_c_shared_utility/umock_c_prod.h"

#include "iothub_devicetwin.h"

/** @brief diagnostic related setting in twin */
typedef struct IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING_TAG
{
    uint32_t diagSamplingPercentage;
} IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING;

/** @brief	Retrieves the given device's E2E diagnostic setting.
*
* @param	serviceClientDeviceTwinHandle	The handle created by a call to the create function.
* @param    deviceId      The device name (id) to retrieve twin info for.
*
* @return	A non-NULL pointer to IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING containing diagnostic setting upon success or NULL upon failure.
*/
MOCKABLE_FUNCTION(, IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING*, IoTHub_E2EDiagnostic_GetSetting, IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, serviceClientDeviceTwinHandle, const char*, deviceId);

/** @brief	Updates (partial update) the given device's twin info.
*
* @param	serviceClientDeviceTwinHandle	The handle created by a call to the create function.
* @param    deviceId                        The device name (id) to retrieve twin info for.
* @param    deviceTwinJson                  DeviceTwin JSon string containing the info (tags, desired properties) to update.
*                                           All well-known read-only members are ignored.
*                                           Properties provided with value of null are removed from twin's document.
*
* @return	A non-NULL pointer to IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING containing diagnostic setting upon success or NULL upon failure.
*/
MOCKABLE_FUNCTION(, IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING*, IoTHub_E2EDiagnostic_UpdateSetting, IOTHUB_SERVICE_CLIENT_DEVICE_TWIN_HANDLE, serviceClientDeviceTwinHandle, const char*, deviceId, IOTHUB_SERVICE_CLIENT_TWIN_E2E_DIAGNOSTIC_SETTING*, diagSetting);


#ifdef __cplusplus
}
#endif

#endif // IOTHUB_SERVICE_DIAGNOSTIC_H
