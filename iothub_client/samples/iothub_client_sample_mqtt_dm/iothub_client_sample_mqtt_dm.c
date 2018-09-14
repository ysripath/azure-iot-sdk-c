// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

//*******************************************************************************
// THIS SAMPLE IS PROVIDED ONLY FOR ILLUSTRATIVE PURPOSES.
// IT RECEIVES MINIMAL MICROSOFT SUPPORT AND SHOULD NOT BE USED FOR PRODUCTION.
//*******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "serializer.h"
#include "iothub_client.h"

#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/lock.h"

#include "iothubtransportmqtt.h"
#include "iothub_device_client.h"

#include "iothub_client_sample_mqtt_dm.h"

#include "parson.h"

/* enum values are in lower case per design */
#define FIRMWARE_UPDATE_STATUS_VALUES \
		waiting1, \
		downloading1, \
		downloadFailed1, \
		downloadComplete1, \
		applying1, \
		applyFailed1, \
		applyComplete1
DEFINE_ENUM(FIRMWARE_UPDATE_STATUS, FIRMWARE_UPDATE_STATUS_VALUES)
DEFINE_ENUM_STRINGS(FIRMWARE_UPDATE_STATUS, FIRMWARE_UPDATE_STATUS_VALUES)


BEGIN_NAMESPACE(Contoso);

DECLARE_STRUCT(firmwareUpdate_t,
	ascii_char_ptr, status
);

typedef enum UPDATE_STATUS{
	current,
	waiting,
	downloading,
	downloadFailed,
	downloadComplete,
	applying,
	applyFailed,
	applyComplete
} UPDATE_STATUS_VALUES;
static inline char *string_UPDATE_STATUS(UPDATE_STATUS_VALUES f)
{
	static char *strings[] = {"current", "waiting", "downloading", "downloadFailed", "downloadComplete", "applying", "applyFailed", "applyComplete" };

	return strings[f];
}
/*firmware: {
	  currentFwVersion: '1.0.0',
	  pendingFwVersion: '',
	  fwUpdateStatus: 'current',
	  fwUpdateSubstatus: '',
	  lastFwUpdateStartTime: '',
	  lastFwUpdateEndTime: ''
	}*/
DECLARE_MODEL(iothubDM_t,
	WITH_REPORTED_PROPERTY(ascii_char_ptr, currentFwVersion),
	WITH_REPORTED_PROPERTY(ascii_char_ptr, pendingFwVersion),
	WITH_REPORTED_PROPERTY(firmwareUpdate_t, fwUpdateStatus),
	//WITH_REPORTED_PROPERTY(firmwareUpdate_t, fwUpdateSubstatus),
	WITH_REPORTED_PROPERTY(ascii_char_ptr, lastFwUpdateStartTime), // UV -  Check datat type for timestamp/date
	WITH_REPORTED_PROPERTY(ascii_char_ptr, lastFwUpdateEndTime) // UV - Check datat type for timestamp/date
);

DECLARE_MODEL(thingie_t,
	//WITH_REPORTED_PROPERTY(iothubDM_t, iothubDM)
	WITH_REPORTED_PROPERTY(iothubDM_t, firmware)
);

END_NAMESPACE(Contoso);


bool reportFlag = false;

typedef struct PHYSICAL_DEVICE_TAG
{
	thingie_t   *iot_device;
	LOCK_HANDLE  status_lock;
	char *new_firmware_URI;
	FIRMWARE_UPDATE_STATUS status;
} PHYSICAL_DEVICE;

typedef struct ARTIK_DEVICE_TAG
{
	char* currentFwVersion; // reported property
	UPDATE_STATUS_VALUES status; // reported property
	char* new_firmware_URI; // desired property
	char* lastFwUpdateStartTime; // reported property
	char* lastFwUpdateEndTime; // reported property
} ARTIK_DEVICE;

typedef struct ARTIK_DEVICE_DESIRED_TAG
{
	char* fwVersion; // desired property
	char* URI; // desired property
	char* checkSum; // desired property
} ARTIK_DEVICE_DESIRED;


static bool set_physical_device_fwupdate_status(PHYSICAL_DEVICE *physical_device, FIRMWARE_UPDATE_STATUS newStatus)
{
	bool retValue;
	if (Lock(physical_device->status_lock) != LOCK_OK)
	{
		LogError("failed to acquire lock");
		retValue = false;
	}
	else
	{
		physical_device->status = newStatus;
		retValue = true;
		if (Unlock(physical_device->status_lock) != LOCK_OK)
		{
			LogError("failed to release lock");
		}
	}
	return retValue;
}

static FIRMWARE_UPDATE_STATUS get_physical_device_fwupdate_status(const PHYSICAL_DEVICE *physical_device)
{
	FIRMWARE_UPDATE_STATUS retValue;

	if (Lock(physical_device->status_lock) != LOCK_OK)
	{
		LogError("failed to acquire lock");
		retValue = waiting;
	}
	else
	{
		retValue = physical_device->status;
		if (Unlock(physical_device->status_lock) != LOCK_OK)
		{
			LogError("failed to release lock");
		}
	}
	return retValue;
}

static int do_firmware_update(void *param)
{
	int   retValue;
	PHYSICAL_DEVICE *physical_device = (PHYSICAL_DEVICE *) param;

	LogInfo("do_firmware_update('%s')", physical_device->new_firmware_URI);
	if (!set_physical_device_fwupdate_status(physical_device, downloading))
	{
		LogError("failed to update device status");
		retValue = -1;
	}
	else
	{
		bool result = device_download_firmware(physical_device->new_firmware_URI);
		if (result)
		{
			if (!set_physical_device_fwupdate_status(physical_device, downloadComplete))
			{
				LogError("failed to update device status");
				retValue = -1;
			}
			else
			{
				LogInfo("starting fw update");
				if (!set_physical_device_fwupdate_status(physical_device, applying))
				{
					LogError("failed to update device status");
					retValue = -1;
				}
				else
				{
					result = device_update_firmware();
					if (result)
					{
						device_reboot();
						retValue = 0;
					}
					else
					{
						if (!set_physical_device_fwupdate_status(physical_device, applyFailed))
						{
							LogError("failed to update device status");
						}
						LogError("firmware update failed during apply");
						retValue = -1;
					}
				}
			}
		}
		else
		{
			LogError("failed to download new firmware image from '%s'", physical_device->new_firmware_URI);
			if (!set_physical_device_fwupdate_status(physical_device, downloadFailed))
			{
				LogError("failed to update device status");
			}
			retValue = -1;
		}
	}
	return retValue;
}


//  Converts the desired properties of the Device Twin JSON blob received from IoT Hub into a Car object.
static ARTIK_DEVICE_DESIRED* parseFromJson(const char* json, DEVICE_TWIN_UPDATE_STATE update_state)
{
	ARTIK_DEVICE_DESIRED* device = malloc(sizeof(ARTIK_DEVICE_DESIRED));
	(void)memset(device, 0, sizeof(ARTIK_DEVICE_DESIRED));


	JSON_Value* root_value = json_parse_string(json);
	JSON_Object* root_object = json_value_get_object(root_value);

	// Only desired properties:
	JSON_Value* fwVersion;
	JSON_Value* fwPackageURI;
	JSON_Value* fwPackageCheckValue;


	fwVersion = json_object_dotget_value(root_object, "firmware.fwVersion");
	fwPackageURI = json_object_dotget_value(root_object, "firmware.fwPackageURI");
	fwPackageCheckValue = json_object_dotget_value(root_object, "firmware.fwPackageCheckValue");

	const char* data = json_value_get_string(fwVersion);
	device->fwVersion = malloc(strlen(data)+1);
	(void)strcpy(device->fwVersion, json_value_get_string(fwVersion));

	device->URI = malloc(strlen(json_value_get_string(fwPackageURI))+1);
	(void)strcpy(device->URI, json_value_get_string(fwPackageURI));

	device->checkSum = malloc(strlen(json_value_get_string(fwPackageCheckValue))+1);
	(void)strcpy(device->checkSum, json_value_get_string(fwPackageCheckValue));

	return device;
}

static void reportedStateCallback(int status_code, void* userContextCallback)
{
	(void)userContextCallback;
	LogInfo("Device Twin reported properties update completed with result: %d", status_code);
}


static void deviceTwinCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* userContextCallback)
{
	LogInfo("deviceTwin CallBack");
	LogInfo("Payload - %s", payLoad);

	if (!reportFlag)
	{

		ARTIK_DEVICE_DESIRED* device = parseFromJson((const char*)payLoad, update_state);
		LogInfo("Desired fw version %s", device->fwVersion);
		LogInfo("URI for downloading firmware - \"%s\"", device->URI);
		LogInfo("Checksum of the firmware %s",device->checkSum);

	}
	else
	{
		reportFlag = false;
	}

}
static void DeviceTwinCallback(int status_code, void* userContextCallback)
{
	(void)(userContextCallback);
	LogInfo("DeviceTwin CallBack: Status_code = %u", status_code);
}

#define FIRMWARE_UPDATE_METHOD_NAME "firmwareUpdate"

#define SERVER_ERROR 500
#define NOT_IMPLEMENTED 501
#define NOT_VALID 400
#define SERVER_SUCCESS 200


static int DeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback)
{
	PHYSICAL_DEVICE *physical_device = (PHYSICAL_DEVICE *) userContextCallback;

	int retValue;
	if (method_name == NULL)
	{
		LogError("invalid method name");
		retValue = NOT_VALID;
	}
	else if ((response == NULL) || (resp_size == NULL))
	{
		LogError("invalid response parameters");
		retValue = NOT_VALID;
	}
	else if (physical_device == NULL)
	{
		LogError("invalid user context callback data");
		retValue = NOT_VALID;
	}
	else if (strcmp(method_name, FIRMWARE_UPDATE_METHOD_NAME) == 0)
	{
		if (get_physical_device_fwupdate_status(physical_device) != waiting)
		{
			LogError("attempting to initiate a firmware update out of order");
			retValue = NOT_VALID;
		}
		else
		{
			if (payload == NULL)
			{
				LogError("passing invalid parameters payload");
				retValue = NOT_VALID;
			}
			else if (size == 0)
			{
				LogError("passing invalid parameters size");
				retValue = NOT_VALID;
			}
			else
			{
				physical_device->new_firmware_URI = malloc(size - 1);
				if (physical_device->new_firmware_URI == NULL)
				{
					LogError("failed to allocate memory for URI parameter");
					retValue = SERVER_ERROR;
				}
				else
				{
					/* remove the quotation around the argument */
					memcpy(physical_device->new_firmware_URI, payload + 1, size - 2);
					physical_device->new_firmware_URI[size - 2] = '\0';

					THREAD_HANDLE thread_apply;
					THREADAPI_RESULT t_result = ThreadAPI_Create(&thread_apply, do_firmware_update, physical_device);
					if (t_result == THREADAPI_OK)
					{
						retValue = SERVER_SUCCESS;
						*response = NULL;
						*resp_size = 0;
					}
					else
					{
						LogError("failed to start firmware update thread");
						retValue = SERVER_ERROR;
					}
				}
			}
		}
	}
	else
	{
		LogError("invalid method '%s'", method_name);
		retValue = NOT_VALID;
	}

	return retValue;
}

//  Converts the Car object into a JSON blob with reported properties that is ready to be sent across the wire as a twin.
static char* serializeToJson(const ARTIK_DEVICE* artik_device)
{
	char* result;
	char* val =  string_UPDATE_STATUS(artik_device->status);
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);	
	// Only reported properties:
	(void)json_object_dotset_string(root_object, "firmware.currentFwVersion", artik_device->currentFwVersion);
	(void)json_object_dotset_string(root_object, "firmware.status", val);
	(void)json_object_dotset_string(root_object, "firmware.new_firmware_URI", artik_device->new_firmware_URI);
	(void)json_object_dotset_string(root_object, "firmware.lastFwUpdateEndTime", artik_device->lastFwUpdateEndTime);
	(void)json_object_dotset_string(root_object, "firmware.lastFwUpdateStartTime", artik_device->lastFwUpdateStartTime);

	result = json_serialize_to_string(root_value);

	json_value_free(root_value);

	return result;
}

static bool send_reported(const ARTIK_DEVICE *artik_device, IOTHUB_CLIENT_HANDLE iotHubClientHandle)
{
	/*time_t rawtime;
		struct tm * timeinfo;

		time ( &rawtime );
		timeinfo = localtime ( &rawtime );

		iot_device->firmware.lastFwUpdateStartTime = asctime (timeinfo);
		iot_device->firmware.lastFwUpdateEndTime = asctime (timeinfo);*/
	/*time_t now;
		time(&now);
		char buf[sizeof "2011-10-08T07:07:09Z"];
		strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
		iot_device->firmware.lastFwUpdateStartTime = buf;
		iot_device->firmware.lastFwUpdateEndTime = buf;*/

	/*serialize the model using SERIALIZE_REPORTED_PROPERTIES */
	char* buffer = serializeToJson(artik_device);

	LogInfo("Serialized object \n %s", buffer);
	/* send the data up stream*/
	(void)IoTHubDeviceClient_SendReportedState(iotHubClientHandle, (const unsigned char*)buffer, strlen(buffer), reportedStateCallback, NULL);
	(void)IoTHubDeviceClient_SetDeviceTwinCallback(iotHubClientHandle, deviceTwinCallback, &artik_device);

	free(buffer);

	return true;
}
static ARTIK_DEVICE* artik_device_new()
{
	ARTIK_DEVICE* dev = malloc(sizeof(ARTIK_DEVICE));
	dev->currentFwVersion = malloc(strlen("0.1")+1);
	(void)strcpy(dev->currentFwVersion, "0.1");
	dev->status = current;
	dev->new_firmware_URI =  malloc(strlen("")+1);
	(void)strcpy(dev->new_firmware_URI, "");
	dev->lastFwUpdateEndTime =  malloc(strlen("")+1);
	(void)strcpy(dev->lastFwUpdateEndTime, "");
	dev->lastFwUpdateStartTime =  malloc(strlen("")+1);
	(void)strcpy(dev->lastFwUpdateStartTime, "");
	return dev;
}
static PHYSICAL_DEVICE* physical_device_new(thingie_t *iot_device)
{
	PHYSICAL_DEVICE *retValue = malloc(sizeof(PHYSICAL_DEVICE));
	if (retValue == NULL)
	{
		LogError("failed to allocate memory for physical device structure");
	}
	else
	{
		retValue->status_lock = Lock_Init();
		if (retValue->status_lock == NULL)
		{
			LogError("failed to create a lock");
			free(retValue);
			retValue = NULL;
		}
		else
		{
			retValue->iot_device = iot_device;
			retValue->status = waiting;
			retValue->new_firmware_URI = NULL;
		}
	}

	return retValue;
}

static void physical_device_delete(PHYSICAL_DEVICE *physical_device)
{
	if (physical_device->new_firmware_URI != NULL)
	{
		free(physical_device->new_firmware_URI);
	}
	if (Lock_Deinit(physical_device->status_lock) == LOCK_ERROR)
	{
		LogError("failed to release lock handle");
	}
	free(physical_device);
}

static void artik_device_delete(ARTIK_DEVICE *dev)
{
	if (dev->currentFwVersion != NULL)
	{
		free(dev->currentFwVersion);
	}
	if (dev->lastFwUpdateEndTime != NULL)
	{
		free(dev->lastFwUpdateEndTime);
	}
	if (dev->lastFwUpdateStartTime != NULL)
	{
		free(dev->lastFwUpdateStartTime);
	}
	if (dev->new_firmware_URI != NULL)
	{
		free(dev->new_firmware_URI);
	}
	free(dev);
}

static int iothub_client_sample_mqtt_dm_run(const char *connectionString, bool traceOn)
{
	LogInfo("Initialize Platform");

	int retValue;
	if (platform_init() != 0)
	{
		LogError("Failed to initialize the platform.");
		retValue = -4;
	}
	else
	{
		if (serializer_init(NULL) != SERIALIZER_OK)
		{
			LogError("Failed in serializer_init.");
			retValue = -5;
		}
		else
		{
			LogInfo("Instantiate the device.");
			thingie_t *iot_device = CREATE_MODEL_INSTANCE(Contoso, thingie_t);
			if (iot_device == NULL)
			{
				LogError("Failed on CREATE_MODEL_INSTANCE.");
				retValue = -6;
			}

			else
			{
				LogInfo("Initialize From Connection String.");
				IOTHUB_CLIENT_HANDLE iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString, MQTT_Protocol);
				if (iotHubClientHandle == NULL)
				{
					LogError("iotHubClientHandle is NULL!");
					retValue = -7;
				}
				else
				{
					LogInfo("Device successfully connected.");
					if (IoTHubClient_SetOption(iotHubClientHandle, "logtrace", &traceOn) != IOTHUB_CLIENT_OK)
					{
						LogError("failed to set logtrace option");
					}

					//PHYSICAL_DEVICE *physical_device = physical_device_new(iot_device);
					ARTIK_DEVICE* artik_device = artik_device_new();
					if (artik_device == NULL)
					{
						LogError("failed to make an artik device callback structure");
						retValue = -8;
					}
					else
					{
						if (IoTHubClient_SetDeviceMethodCallback(iotHubClientHandle, DeviceMethodCallback, artik_device) != IOTHUB_CLIENT_OK)
						{
							LogError("failed to associate a callback for device methods");
							retValue = -9;
						}
						else
						{
							bool keepRunning = send_reported(artik_device, iotHubClientHandle);
							if (!keepRunning)
							{
								LogError("Failed to send initia device reported");
								retValue = -10;
							}
							else
							{
								LogInfo("Reported properties sent successfully");
								reportFlag = true;
								while (keepRunning)
								{

								}

								/*FIRMWARE_UPDATE_STATUS oldStatus = get_physical_device_fwupdate_status(physical_device);
								while (keepRunning)
								{
									FIRMWARE_UPDATE_STATUS newStatus = get_physical_device_fwupdate_status(physical_device);

									// send reported only if the status changes
									if (newStatus != oldStatus)
									{
										oldStatus = newStatus;
										keepRunning = send_reported(physical_device, iotHubClientHandle);
									}
									ThreadAPI_Sleep(1000);
								}
								retValue = 0;*/
							}
						}
						//physical_device_delete(physical_device);
						artik_device_delete(artik_device);
					}
					IoTHubClient_Destroy(iotHubClientHandle);
				}
				//DESTROY_MODEL_INSTANCE(iot_device);
			}
			serializer_deinit();
		}
		platform_deinit();
	}

	return retValue;
}

int main(int argc, char *argv[])
{
	int   exitCode = 0;
	bool  isService = true;
	bool  traceOn = false;
	char *connectionString = NULL;

	for (int ii = 1; ii < argc; ++ii)
	{
		if (0 == strcmp(argv[ii], "-console"))
		{
			isService = false;
		}
		else if (0 == strcmp(argv[ii], "-cs"))
		{
			++ii;
			if (ii < argc)
			{
				if (mallocAndStrcpy_s(&connectionString, argv[ii]) != 0)
				{
					LogError("failed to allocate memory for connection string");
					exitCode = -1;
				}
			}
		}
		else if (0 == strcmp(argv[ii], "-logging"))
		{
			traceOn = true;
		}
	}

	if (exitCode == 0)
	{
		if (connectionString == NULL)
		{
			connectionString = device_get_connection_string();
			//connectionString = "HostName=artikHub.azure-devices.net;DeviceId=device_1;SharedAccessKey=94E0wsllwoH4NMNebdI1SSDtbOoF1aVqO6jyiytkyE0=";
		}
		printf("Connection string %s\n", connectionString);
		if (connectionString == NULL)
		{
			LogError("connection string is NULL");
			exitCode = -2;
		}
		else
		{
			if (isService)
			{
				traceOn = false;
				if (device_run_service() == false)
				{
					LogError("Failed to run as a service.");
					exitCode = -3;
				}
			}

			if (exitCode == 0)
			{
				exitCode = iothub_client_sample_mqtt_dm_run(connectionString, traceOn);
			}

			free(connectionString);
		}
	}

	LogInfo("Exit Code: %d", exitCode);
	return exitCode;
}
