// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Sample IoT Plug and Play device app for X.509 certificate attestation
//
// Daisuke Nakahara 
//

#ifndef _DPS
#define _DPS

#include "impinj_reader_r700.h"
#include "iothub_op.h"
#include "iothub_device_client_ll.h"
#include "azure_prov_client/iothub_security_factory.h"
#include "azure_prov_client/prov_device_ll_client.h"
#include "azure_prov_client/prov_transport_mqtt_client.h"
#include "azure_prov_client/prov_security_factory.h"
#include "azure_c_shared_utility/strings.h"

typedef struct DPS_CLIENT_CONTEXT_TAG
{
    unsigned int sleep_time;
    char* iothub_uri;
    char* access_key_name;
    char* device_key;
    char* device_id;
    bool registration_complete;
    PAPP_CONTEXT appContext;
} DPS_CLIENT_CONTEXT;

CLIENT_LL_HANDLE ProvisionDevice(const char* scopeId, const char* deviceId, const char* deviceKey, const char* modelId, APP_CONTEXT* appContext);
CLIENT_LL_HANDLE ProvisionDeviceX509(const char* scopeId, const char* modelId, APP_CONTEXT* appContext);

#endif /* _DPS */