// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>

#include "azure_c_shared_utility/platform.h"

#include "../../../certs/certs.h"

#include "prov_service_client/provisioning_service_client.h"

static bool g_use_trace = true;

#ifdef USE_OPENSSL
static bool g_use_certificate = true;
#else
static bool g_use_certificate = false;
#endif //USE_OPENSSL

int main()
{
    int result = 0;

	const char* connectionString = "HostName=artikDPS.azure-devices-provisioning.net;SharedAccessKeyName=provisioningserviceowner;SharedAccessKey=BdkkdHSZ53kL9KoHsEpJ4xcY/RqVmIDn/PogoVWWaYA=";
	const char* groupId = "artikgroup";
	const char* signingCertificate = "-----BEGIN CERTIFICATE-----\
MIIFOjCCAyKgAwIBAgIJAK4hDNI3ZymfMA0GCSqGSIb3DQEBCwUAMCoxKDAmBgNV\
BAMMH0F6dXJlIElvVCBIdWIgQ0EgQ2VydCBUZXN0IE9ubHkwHhcNMTgwOTI2MjEz\
MzExWhcNMTgxMDI2MjEzMzExWjAqMSgwJgYDVQQDDB9BenVyZSBJb1QgSHViIENB\
IENlcnQgVGVzdCBPbmx5MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA\
xTuAE2punN2C1jiF8LYGF3OTlv/6BBoYQzjh9EiXr4PxT0Ize9da8xFNDtxDQwAE\
T+fjH3c3yQXzHkFu5jt00/S9Rq7r7YUYeGPR1LpErNsr15Ch/lgYP0eLAbow9IAD\
EQVkyYaBoFzwpktX4XC07DMHclXuwRHgV4SUYgXj1/S/jnWDSd+Qfr/oRxk3NAi8\
6F4Yz7qGxldSwheIO+BNcfN9aQK/c/sFurB1QJAuAFzN5VUvzV+ZOtV3gV6Dm+iC\
1z3Z/2nEzvVyQ2qBFkyHT2ySZTm9pkfjtpU9xeFfcHi5YKQt8hBYeJmPEwe9kHzj\
YtrJv4IR1DS5w9T0ITFvT+vNDtF44RdqYZ8YNNZZ64u+DhjRj2kVc6ulfwZ7rpG8\
KppgBMKTfKNPyC9JpLIXC4E/Xq3LwfkFX1a+Nie5FnG9LYqFlcA3kjI6PcARIA9X\
xSLidz/dijoNlboKMqeRuO3ihr1gKVtEYiNSlkdFnT5Y7qsnVIw6P9YXLFsHbjAx\
uwpCqseeDEwY10HTOThq7u8XRGCk87zDsbj66pqCUBPaUWG4VCZUcYGmlXVD0zh4\
ODNd3IJTaFi/s3Xzws5zqS1dZwMZiAtZbDXLrnSioxhxjTmg3SQVBBLlHiFLcMwy\
j/Cn2Pemj385kJEKP9wKBS2rnU3xXWH/yxkbzlt7rx0CAwEAAaNjMGEwHQYDVR0O\
BBYEFDW0Z/s39lelE0vJcg002UsTG1FeMB8GA1UdIwQYMBaAFDW0Z/s39lelE0vJ\
cg002UsTG1FeMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgGGMA0GCSqG\
SIb3DQEBCwUAA4ICAQDC0iTVJz80u83SjqGNZN+gu/bH6fLNgRCbEW8Q50ZhnKIz\
hkDduc5SVlCAGfyofv2uRU6SKUtm01pSoqMECCfMaBJ4bC0AEo/sXTnV+TwUtS8B\
iCNeSIgbkGC6yNGJp5ZFAoTN/Npabkw9mcAmI2kgnz+dOug9TuznvpSuMljqQCk6\
FAoZPvOqvqRMBThuB8vepQo015n1hUlziUb49ct0zvObHEgcIbZvNHgVHT0WCJcM\
4XxtlmPKd+TyQkBYj+ERsSAbiPL+Jgdp5Fi7P5GDyIIQc/4FkSKZHEj7QLJCGV3Y\
lvpmbXhipEnNNHRIPUQjUnK6ikizlTAKO8HF1aKLjx24CiZQfBW6WZF3ynlG48Nw\
AtbjYHo396UV/IPzSKm83gD/LaLJfnSzStPPRvWy2/IR0e/8paJcA9Wyls1Z23y7\
V2AIxYQV7CRhHl7aL1ivJP/zTmxTgSCxnmYbEW0ROwtP2dzwnJJXicysKoOnNvzS\
p1+sbmUdQLdFQb66EFHLSWSZIi0tY9iZpmafaJ0zsU1lsLDSz/aLy+ILcSwA/hDG\
DlFA0JtZoXGZo3QS4NjfB8dDoy+4Psg+4sPf85gGSUBBded7ZFQKGMDP0fTK2c3F\
2oK9xXFS0yQ+s/21jCbumzzDU3f4sVU8CyfImZzXOfzeUc5FU2ElbzE4WykxqQ==\
-----END CERTIFICATE-----";

	/*const char* signingCertificate = "-----BEGIN PRIVATE KEY-----\
	MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDEVYk0UYB5AzcK\
	qFAayI+q3ramK8wAFu6oxWKRqcteaczBppn0HQDWZFccd/yVOThw3rs4Cj25hzQJ\
	w2PQDcBm1CoJhLDCjmgg5tjpW9+pCu9nCGUq8MDPhsQ3wsW8FA2NMWDKZ4hzNde9\
	YgFyFuzt+C6V4v+jhZ1wPxB53YliKfWGUraKQN/lRC1K4+1NCw446nMcjnqsUVaU\
	fdZFhe/8Qi3GXhIntufDpenV7sjqDQ6RT5DUBTL6kjLJpwG7KmDkUrDQhf5vFEII\
	EWDBG5Ucf1jsHJLKlEgkNSi/kNoRX1Q4+NENHclFHgf3oGteUszYDbrdYvberTo2\
	bmUXSW6xAgMBAAECggEABScZir342TQ0ZZs1Pv60ho+By6qql/YCHXvQsnH6XGf6\
	b7qnn7iDJR1NC/o6LtBposy1rSrXo97ghsESBYvW2NwInmdVHEYQhYyJdNZscEtR\
	LSsTPKVE6i1nSO9bsSv1jxcUqzs8YYmwN9AyqYPgg/5XeqeSfgAruZ5uGESwLltP\
	wByqhF+MWdUDjBtOYJjHqlBLqzG7Pdvk7OrIrXSswge73509jwPsuevYZdwH/E7U\
	FZstLHstiliDaO/C/X2STWKKKNlj9HoS0ymDAW6FkSg/Wqs6uhb22Svyjpfd67ti\
	67hPiDeBZC50kuRNA/NAA8d3cka3FTsMeG++sSrX4QKBgQDy2UHeyOQrJvxvLE7e\
	fwjHKAXRz3UPXjiFUL57bjjU0tI+62ZskIFDbudjilkq4I52so09u4775EOBFhoo\
	TjX232biYhqDd05sCBHgAZ73q1hj3U1NDtSy8ahUQFNn9MQqBZy7Jdg7hz75IaAS\
	H25B1VM92w4bJMil+/s+4Y1kvwKBgQDO92winVV0PXtVK9m/o0Tq6/y+jJfdrEpp\
	jZ0uv+GdJAGCmVAt4zGDT+V48Z92sY80YdiIulglmGIUUEAPan1gDbndZ/AfmgBI\
	t+knAfBC/yuk2Nf+sMfHL8CqccgV5HLhs3ledRuj0PH/KAeBsSkDamV3/KbQbC4m\
	KlfklfPYjwKBgEtYJnylXU0aGmWvnIShayrG+w8+SzZKaETMaVzINK+q/PnydOOn\
	7cLrLLUQXlvUMb8X5IRLpa/3AQ6SuejZYxrF8xi3kTxKjrUrx8f1GMoEijbpmSsY\
	N9uQ6EXDc10kbpwPA7J4ql7Ftj77NLuKrt2T/vCI/xZi0jHVPP0kY4bnAoGAVZAo\
	nm7ZI0M/t8h3Lyj6lvyU8toA9t4BrX2kW+1sAqEeFrX3VeE1WQow3j1WJaXmhEtn\
	T69qPbCv66H5ueXWi11hV81eklICiA2wUDYW9Du1+WLEeUDwdsKhLlX32EUn0XN3\
	W7uif6kkQs5zvARX5leYN3C2LjhrI9Ahohj0RLUCgYABqYJSs2gm9hIrB/xcbEfT\
	3+OteqwLnffUjBtGvD5i603yBpdktrPQ95qRrFt0hdQulQ6OKMcj1faeqRU+9ku3\
	LbQhsQ1blUYQE+u2AzzP3JqsoA+l9BShOQ/uwJoDEHvStDaz/4UU51zQryhGOA8K\
	j438kjUUI//7gGfgytRuMQ==\
	-----END PRIVATE KEY-----";*/

    const char* tags = "{\"key\":\"value\"}";

    PROVISIONING_SERVICE_CLIENT_HANDLE prov_sc;
    ATTESTATION_MECHANISM_HANDLE am_handle;
    ENROLLMENT_GROUP_HANDLE eg_handle;
    INITIAL_TWIN_HANDLE twin_handle;

    printf("Starting Enrollment Group sample...\n");

    /* ---Create a Provisioning Service Client Handle */

    printf("Connecting to the Provisioning Service...\n");

    /*This function must be called before anything else so that sockets work*/
    platform_init();

    /* This creates a handle that will be used to represent a connection to the
    Device Provisioning Service indicated by the connection string given. */
    prov_sc = prov_sc_create_from_connection_string(connectionString);

    /* ---Optionally set connection options--- */
    if (g_use_trace)
    {
        prov_sc_set_trace(prov_sc, TRACING_STATUS_ON);
    }
    if (g_use_certificate)
    {
        prov_sc_set_certificate(prov_sc, certificates);
    }

    /* ---Create an Enrollment Group structure--- */
    printf("Creating an Enrollment Group structure...\n");

    am_handle = attestationMechanism_createWithX509SigningCert(signingCertificate, NULL);
    eg_handle = enrollmentGroup_create(groupId, am_handle);

    /* ---Create the Enrollment Group on the Provisioning Service--- */
    printf("Creating an Enrollment Group on the Provisioning Service...\n");

    /*Note that after a successful create, the pointer of the given model
    is updated to contain new information generated by the Provisioning Service*/
    prov_sc_create_or_update_enrollment_group(prov_sc, &eg_handle);

    /* ---Retrieve an Enrollment Group on the Provisioning Service--- */
    printf("Retrieving an Enrollment Group from the Provisioning Service...\n");

    /*Note that in this context, doing a "get" call is a useless since the "create or update" call
    above already updated "eg_handle", and no changes have been made since. This is just to
    show you how a "get" would be performed */
    enrollmentGroup_destroy(eg_handle);
    prov_sc_get_enrollment_group(prov_sc, groupId, &eg_handle);

    /* ---Update an Enrollment Group on the Provisioning Service ---*/
    printf("Updated an Individual Enrollment structure\n");

    //in this example we'll add an initial twin state
    twin_handle = initialTwin_create(tags, NULL);
    enrollmentGroup_setInitialTwin(eg_handle, twin_handle);

    /* This is the same call as we used for creation. However, because we already created the
    enrollment on the Provisioning Service, this call will now update the already existing one */
    prov_sc_create_or_update_enrollment_group(prov_sc, &eg_handle);

    /* ---Delete an Enrollment Group on the Provisioning Service--- */
    printf("Deleting an Enrollment Group on the Provisioning Service...\n");

    //prov_sc_delete_enrollment_group(prov_sc, eg_handle);

    /* ---Clean up handles--- */
    enrollmentGroup_destroy(eg_handle);
    prov_sc_destroy(prov_sc);
    platform_deinit();

    return result;
}
