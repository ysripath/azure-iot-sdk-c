// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/utsname.h>

#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"

#include "iothub_client_sample_mqtt_dm.h"


#define NEW_FW_ARCHIVE "/root/newfirmware.zip"


/* the following files will be placed by the 'installer'
   o- /usr/share/iothub_client_sample/firmware_update - this is the executable
   o- /usr/share/iothub_client_sample/iothub_client_sample_firmware_update.service - this is the service controller
   o- /usr/share/iothub_client_sample/.device_connection_string - file containing one line only, the device connection string
   o- /lib/systemd/system/iothub_client_sample_firmware_update.service - symbolic link to /usr/share/iothub_client_sample/iothub_client_sample_firmware_update.service
   o- /etc/systemd/system/multi-user.target.wants/iothub_client_sample_firmware_update.service - symbolic link to /usr/share/iothub_client_sample/iothub_client_samplefirmware_update.service
*/
#define CONNECTION_STRING_FILE_NAME "/usr/share/iothub_client_sample/.device_connection_string"


static int _system(const char *command)
{
    return 1;
}

static int install_zip_package(void)
{
    return 1;
}

static int prepare_to_flash(const char *fileName)
{
    return 1;
}

static char* read_string_from_file(const char *fileName)
{
    char *retValue;
    FILE *fd = fopen(fileName, "rx");

    if (fd == NULL)
    {
        LogError("failed to open file '%s'", fileName);
        retValue = NULL;
    }
    else
    {
        if (fseek(fd, 0L, SEEK_END) != 0)
        {
            LogError("failed to fined the end of file('%s')", fileName);
            retValue = NULL;
        }
        else
        {
            long size = ftell(fd);
            if ((size <= 0) || (fseek(fd, 0L, SEEK_SET) != 0))
            {
                LogError("failed to rewind the file");
                retValue = NULL;
            }
            else
            {
                retValue = malloc((size + 1) * sizeof(char));
                if (retValue == NULL)
                {
                    LogError("failed to allocate buffer for data");
                }
                else
                {
                    char *result = fgets(retValue, size, fd);
                    if (result == NULL)
                    {
                        LogError("fgets failed");
                        free(retValue);
                        retValue = NULL;
                    }
                    else
                    {
                        retValue = result;
                    }
                }
            }
        }
        fclose(fd);
    }
    return retValue;
}

//----------------------------------------------------------------------------------------------------------------------------
char *device_get_firmware_version(void)
{
    return "0001";
}

char* device_get_connection_string(void)
{
    return read_string_from_file(CONNECTION_STRING_FILE_NAME);
}

bool device_download_firmware(const char *uri)
{
	int temp = rand() % (25 + 1 - 10) + 10;
	sleep(temp);
    return true;
}

void device_reboot(void)
{
	int temp = rand() % (30 + 1 - 15) + 15;
	sleep(temp);
}

bool device_update_firmware(void)
{
	int temp = rand() % (15 + 1 - 10) + 10;
	sleep(temp);
    return true;
}

bool device_run_service(void)
{
    return true;
}
