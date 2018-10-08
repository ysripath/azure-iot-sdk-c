#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

char *device_get_firmware_version(void)
{
	return "0001";
}

#define RAND_MAX_DELAY 2
#define RAND_MIN_DELAY 1

bool device_download_firmware(const char *uri)
{
	int temp = rand() % (RAND_MAX_DELAY + 1 - RAND_MIN_DELAY) + RAND_MIN_DELAY;
	sleep(temp);
	return true;
}

void device_reboot(void)
{
	int temp = rand() %  (RAND_MAX_DELAY + 1 - RAND_MIN_DELAY) + RAND_MIN_DELAY;
	sleep(temp);
}

bool device_update_firmware(void)
{
	int temp = rand() %  (RAND_MAX_DELAY + 1 - RAND_MIN_DELAY) + RAND_MIN_DELAY;
	sleep(temp);
	return true;
}

bool device_run_service(void)
{
	return true;
}
