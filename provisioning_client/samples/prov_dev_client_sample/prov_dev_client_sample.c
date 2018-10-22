// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// CAVEAT: This sample is to demonstrate azure IoT client concepts only and is not a guide design principles or style
// Checking of return codes and error values shall be omitted for brevity.  Please practice sound engineering practices 
// when writing production code.

#include <stdio.h>
#include <stdlib.h>
#include "customhsm.h"
#include "iothub.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/http_proxy_io.h"

#include "azure_prov_client/prov_device_client.h"
#include "azure_prov_client/prov_security_factory.h"
#include "iothub_client_options.h"
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

//
// The protocol you wish to use should be uncommented
//
#define SAMPLE_MQTT
//#define SAMPLE_MQTT_OVER_WEBSOCKETS
//#define SAMPLE_AMQP
//#define SAMPLE_AMQP_OVER_WEBSOCKETS
//#define SAMPLE_HTTP

#ifdef SAMPLE_MQTT
#include "iothubtransportmqtt.h"
#include "azure_prov_client/prov_transport_mqtt_client.h"
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
#include "iothubtransportmqtt_websockets.h"
#include "azure_prov_client/prov_transport_mqtt_ws_client.h"
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
#include "iothubtransportamqp.h"
#include "azure_prov_client/prov_transport_amqp_client.h"
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
#include "iothubtransportamqp_websockets.h"
#include "azure_prov_client/prov_transport_amqp_ws_client.h"
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
#include "iothubtransportmqtt.h"
#include "azure_prov_client/prov_transport_http_client.h"
#endif // SAMPLE_HTTP

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

// This sample is to demostrate iothub reconnection with provisioning and should not
// be confused as production code

DEFINE_ENUM_STRINGS(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);
DEFINE_ENUM_STRINGS(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

static const char* global_prov_uri = "global.azure-devices-provisioning.net";
static const char* id_scope = "0ne0002B21C";

static bool g_use_proxy = false;
static const char* PROXY_ADDRESS = "127.0.0.1";

#define PROXY_PORT                  8888
#define MESSAGES_TO_SEND            2
#define TIME_BETWEEN_MESSAGES       2

static const char* x509cert =  "-----BEGIN CERTIFICATE-----\r\n"
							   "MIIDcjCCAlqgAwIBAgIJAOPcxWjzOXTkMA0GCSqGSIb3DQEBCwUAME4xCzAJBgNV\r\n"
							   "BAYTAnVzMQswCQYDVQQIDAJ1czELMAkGA1UEBwwCdXMxCzAJBgNVBAoMAnVzMQsw\r\n"
							   "CQYDVQQLDAJ1czELMAkGA1UEAwwCdXMwHhcNMTgxMDE2MjIzNzA5WhcNMTgxMTE1\r\n"
							   "MjIzNzA5WjBOMQswCQYDVQQGEwJ1czELMAkGA1UECAwCdXMxCzAJBgNVBAcMAnVz\r\n"
							   "MQswCQYDVQQKDAJ1czELMAkGA1UECwwCdXMxCzAJBgNVBAMMAnVzMIIBIjANBgkq\r\n"
							   "hkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxFWJNFGAeQM3CqhQGsiPqt62pivMABbu\r\n"
							   "qMVikanLXmnMwaaZ9B0A1mRXHHf8lTk4cN67OAo9uYc0CcNj0A3AZtQqCYSwwo5o\r\n"
							   "IObY6VvfqQrvZwhlKvDAz4bEN8LFvBQNjTFgymeIczXXvWIBchbs7fguleL/o4Wd\r\n"
							   "cD8Qed2JYin1hlK2ikDf5UQtSuPtTQsOOOpzHI56rFFWlH3WRYXv/EItxl4SJ7bn\r\n"
							   "w6Xp1e7I6g0OkU+Q1AUy+pIyyacBuypg5FKw0IX+bxRCCBFgwRuVHH9Y7BySypRI\r\n"
							   "JDUov5DaEV9UOPjRDR3JRR4H96BrXlLM2A263WL23q06Nm5lF0lusQIDAQABo1Mw\r\n"
							   "UTAdBgNVHQ4EFgQU3CzGViczKytMuGGC2XjYQ7iElmcwHwYDVR0jBBgwFoAU3CzG\r\n"
							   "ViczKytMuGGC2XjYQ7iElmcwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsF\r\n"
							   "AAOCAQEADFuWsLSrLUSiLz9bMIfs3sCMuNM17t8/FNSwrsxgIq+Dly1Kj05NIxCw\r\n"
							   "w+SOc/1DhiyhbBCPVDI2kYX8HQO9Hvlt2WdlkRHUUx1EWiKFvLHfvHeUHFJqlvM3\r\n"
							   "qrG0fqdpamsHsy1Imefrqs+fsK8Pl227gAxfef0/JXdeOirz/In9kD764QeJwT+L\r\n"
							   "BNqJt74Zlwh5USOtp5TYtnwclxDmm4xkgRrcyIw9XCzYbJT/VV22nJ3tH7TDYnQ2\r\n"
							   "vKwW9cDJRhp/Ahkm+nGCvSNhHdRe3E1FOr6pehvzh1EzkfnXg0zvBqMTwwgN/R1X\r\n"
							   "b8lCa0LDW/xcQx3vqazS7iIdqISbog==\r\n"
							   "-----END CERTIFICATE-----\r\n"
							   "-----BEGIN CERTIFICATE-----\r\n"
							   "MIIDJTCCAg0CCQD5mSgFJcN3/jANBgkqhkiG9w0BAQsFADBOMQswCQYDVQQGEwJ1\r\n"
							   "czELMAkGA1UECAwCdXMxCzAJBgNVBAcMAnVzMQswCQYDVQQKDAJ1czELMAkGA1UE\r\n"
							   "CwwCdXMxCzAJBgNVBAMMAnVzMB4XDTE4MTAxODIxNDUzOVoXDTIxMDgwNzIxNDUz\r\n"
							   "OVowWzELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\r\n"
							   "GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDEUMBIGA1UEAwwLYXJ0aWtkZXZpY2Uw\r\n"
							   "ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDvxYH2adih/rAZXBT4jdQG\r\n"
							   "74LKOun3QaF++S7ws1JECabsKBfvFGipxpn12WSgrg3LF1wV3iU4gvnrxyEXwcNs\r\n"
							   "4Y0sIiu+U3pTnBQ5QUP2GliXEMemH9YJoq/EJpQqjrvrYX+hx78CdFiDtK/wwHkX\r\n"
							   "pW2RXnqKjaZso5FmCYo5wUDklGYzALIKjy5gYhZUlp4L3xq7G6lHkHO1qzuDHyqz\r\n"
							   "iA5pu1KxXaqd395jjvyfY0EfLKV7oO1eZterP8GbsXxD2KLZiEE1uFKpRbl2ZOfP\r\n"
							   "uu6FvY0cez0qMjWOT3CRvGWYbBmARRDIFZSAax78L8ofBguO9BZEii2B7+62C73j\r\n"
							   "AgMBAAEwDQYJKoZIhvcNAQELBQADggEBAAFo/LCJVkRIklhNgKdD9gRYQGzqxHqy\r\n"
							   "KjuZUkrKOYyx1DNRnKMC5c5ZZhsmsi+FU7aNIiqKGWaVleEBl10GX2iq0/Q+NS7E\r\n"
							   "vW23C3TbYAZ+k9ZS7/eXlDWpwdb5KNh68sdbVPNdLjEyRYhZC/6hlQKgfI9IA+5i\r\n"
							   "OTw3QfJBvp6iLE8CtKYLNzRiVOgrEKwcubwodZGg9YOfdGtc5+qyJlmMIUDkXhqv\r\n"
							   "VZVC7dR3svw2EIpY0VAV7AJ3H5Kd65PXu7stgJEqBjdih9/mgHpZlzTycR2Z/ucO\r\n"
							   "wvlqdZ3qnUQd78GFVe03I5EXKQj5BOuHbfrEaO0KUjUq9mnSggGVr+Y=\r\n"
							   "-----END CERTIFICATE-----\r\n";

static const char* x509privatekey ="-----BEGIN RSA PRIVATE KEY-----\r\n"
		"MIIEpAIBAAKCAQEA78WB9mnYof6wGVwU+I3UBu+Cyjrp90Ghfvku8LNSRAmm7CgX\r\n"
		"7xRoqcaZ9dlkoK4NyxdcFd4lOIL568chF8HDbOGNLCIrvlN6U5wUOUFD9hpYlxDH\r\n"
		"ph/WCaKvxCaUKo6762F/oce/AnRYg7Sv8MB5F6VtkV56io2mbKORZgmKOcFA5JRm\r\n"
		"MwCyCo8uYGIWVJaeC98auxupR5Bztas7gx8qs4gOabtSsV2qnd/eY478n2NBHyyl\r\n"
		"e6DtXmbXqz/Bm7F8Q9ii2YhBNbhSqUW5dmTnz7ruhb2NHHs9KjI1jk9wkbxlmGwZ\r\n"
		"gEUQyBWUgGse/C/KHwYLjvQWRIotge/utgu94wIDAQABAoIBAQCD2eHA5q3ZT1v5\r\n"
		"ZhimIiARfE449TLJ5E5xq4ezl3jKr2Aah2W59Egl7G+nsVdTwMMXBo59+/4N2ICk\r\n"
		"toktLkFQM6xVM6lIbVFcfvzkC07bG9vRnsmbQfCKterf+7MIwBr4slHkXhpcuCwg\r\n"
		"qREDXGR6q17YTNMoNALrxPrk2KQpY1rXEpPtq/WgU9xX9HEhn1p4l4nQWCWO3NP1\r\n"
		"paoVzCSihc81XXTJU95GHmXk6hNCLvOrGmxzLCEfavyfqPZRO3Lg29pAsb9xLId0\r\n"
		"FFev82hZiLuPwmvANGLAS2ucxYn3vtWWGZXZikZQcgj5AarGasc4vj4DRC56eNWs\r\n"
		"/7waOO4xAoGBAPf3uRcbY84lhzOrp24uA11tdgxrLTIkGXl+EkR41BGFZlNgqfGU\r\n"
		"yoM06bu1cbAPsP8eCYN5i8CMw5u4zYBS/D7xopj8BcdrXP9OD/JhLtMVTvc+H3Sz\r\n"
		"qpVCTwqyfPNv5wZ7+gRBtuzurklufjxAv5FiUHlj+HrPDdO8IuX6ZRIJAoGBAPeJ\r\n"
		"0WDc+2f5OQQ7o4NnfWijy3FDfmNtLmcv8ybg4Po0X1ybxR+DW0nVT+wsZF/m7o9N\r\n"
		"rQcL429TBLpu2CHiso28mdHF11Fmp0VYEmIMhBh8gCyqYGCTYLTURgwxHnRfIypw\r\n"
		"k8gtsqyHLdEIdYOqpFeKfBz43eqzMq/sj7PXJBuLAoGBANrhDr9HzRR1ad+BQJxv\r\n"
		"/0ZjzdTucgmVdesZDpTkNwV0RDuK0tfM5+ljNoIbikvHvgujfvBPxL1lQ3DccoE4\r\n"
		"PzJsZoM1ywAZVBCD3m2rvQJUB80UR/3ibcusqUqe/M6BU2MU4j8JpqeDk9J7tvAG\r\n"
		"k1KIftJ4HSiCRglFQ2TPXpHJAoGATlsAvQGWTyYzpyRdsPWsW9glSNgNhNmFq8Ig\r\n"
		"3LioXmr/mKfyMPR6jBeKsf6nUzgdYZ073RQlTRLhV0ZJAgjpbjY6Fo3ih3DQGAI1\r\n"
		"53WAxuN6CylvUoK0ROlAtxFBS6Ll1cRG80GL4lLz7MwZrrDwTomWmfEpBebtb4SZ\r\n"
		"RFsgodsCgYBLTXJyJRITQEn2D6HDGjmlxx26Vekh2d84hrKA4yKxQK3a5LnNmDEs\r\n"
		"x2tKBqAB6hEAqCt3Jub+WJK98BhdT6U3EhJzZGT0a7TZ2C5ZlbXc0+BUcLs+zD7m\r\n"
		"Ww3TaSrSKNe9jh/ckQ0EvM6COEunlZr1OROismGOIfr5PJZXTBSMvQ==\r\n"
		"-----END RSA PRIVATE KEY-----\r\n";


static void registation_status_callback(PROV_DEVICE_REG_STATUS reg_status, void* user_context)
{
    (void)user_context;
    (void)printf("Provisioning Status: %s\r\n", ENUM_TO_STRING(PROV_DEVICE_REG_STATUS, reg_status));
}

static void register_device_callback(PROV_DEVICE_RESULT register_result, const char* iothub_uri, const char* device_id, void* user_context)
{
    (void)user_context;
    if (register_result == PROV_DEVICE_RESULT_OK)
    {
        (void)printf("\r\nRegistration Information received from service: %s, deviceId: %s\r\n", iothub_uri, device_id);
    }
}

int main()
{
    SECURE_DEVICE_TYPE hsm_type;
    //hsm_type = SECURE_DEVICE_TYPE_TPM;
	hsm_type = SECURE_DEVICE_TYPE_X509;
	//hsm_type = SECURE_DEVICE_TYPE_SYMMETRIC_KEY;

    // Used to initialize IoTHub SDK subsystem
    (void)IoTHub_Init();
    (void)prov_dev_security_init(hsm_type);
    
    HTTP_PROXY_OPTIONS http_proxy;
    PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION prov_transport;

    memset(&http_proxy, 0, sizeof(HTTP_PROXY_OPTIONS));

    // Protocol to USE - HTTP, AMQP, AMQP_WS, MQTT, MQTT_WS
#ifdef SAMPLE_MQTT
    prov_transport = Prov_Device_MQTT_Protocol;
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    prov_transport = Prov_Device_MQTT_WS_Protocol;
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    prov_transport = Prov_Device_AMQP_Protocol;
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    prov_transport = Prov_Device_AMQP_WS_Protocol;
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    prov_transport = Prov_Device_HTTP_Protocol;
#endif // SAMPLE_HTTP

    printf("Provisioning API Version: %s\r\n", Prov_Device_GetVersionString());

    if (g_use_proxy)
    {
        http_proxy.host_address = PROXY_ADDRESS;
        http_proxy.port = PROXY_PORT;
    }

    PROV_DEVICE_RESULT prov_device_result = PROV_DEVICE_RESULT_ERROR;
    PROV_DEVICE_HANDLE prov_device_handle;
    if ((prov_device_handle = Prov_Device_Create(global_prov_uri, id_scope, prov_transport)) == NULL)
    {
        (void)printf("failed calling Prov_Device_Create\r\n");
    }
    else
    {
        if (http_proxy.host_address != NULL)
        {
            Prov_Device_SetOption(prov_device_handle, OPTION_HTTP_PROXY, &http_proxy);
        }

        //bool traceOn = true;
        //Prov_Device_SetOption(prov_device_handle, PROV_OPTION_LOG_TRACE, &traceOn);
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
        // Setting the Trusted Certificate.  This is only necessary on system with without
        // built in certificate stores.
		//Prov_Device_SetOption(prov_device_handle, OPTION_TRUSTED_CERT, certificates);
#endif // SET_TRUSTED_CERT_IN_SAMPLES

		Prov_Device_SetOption(prov_device_handle, PROV_REGISTRATION_ID, "artikgroup");
		Prov_Device_SetOption(prov_device_handle, OPTION_X509_CERT, x509cert);
		Prov_Device_SetOption(prov_device_handle, OPTION_X509_PRIVATE_KEY, x509privatekey);
		bool traceOn = true;
		Prov_Device_SetOption(prov_device_handle, OPTION_LOG_TRACE, &traceOn);
		prov_device_result = Prov_Device_Register_Device(prov_device_handle, register_device_callback, NULL, registation_status_callback, NULL);

        (void)printf("\r\nRegistering... Press enter key to interrupt.\r\n\r\n");
        (void)getchar();

        Prov_Device_Destroy(prov_device_handle);
    }
    prov_dev_security_deinit();

    // Free all the sdk subsystem
    IoTHub_Deinit();

    (void)printf("Press enter key to exit:\r\n");
    (void)getchar();

    return 0;
}
