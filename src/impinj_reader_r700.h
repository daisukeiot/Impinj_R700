// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifndef R700_READER_H
#define R700_READER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

#ifdef PNPBRIDGE
#include <pnpadapter_api.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <parson.h>
#include "curl_wrapper/curl_wrapper.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/const_defines.h"
#include "azure_c_shared_utility/threadapi.h"
#include <iothub_device_client_ll.h>
#include <iothub_module_client_ll.h>

// Map IoT Plug and Play Bridge types to generic SDK types
#ifdef USE_EDGE_MODULES
#define CLIENT_LL_HANDLE IOTHUB_MODULE_CLIENT_LL_HANDLE
#define PnpBridgeClient_SendReportedState(iotHubClientHandle, reportedState, size, reportedStateCallback, userContextCallback) IoTHubModuleClient_LL_SendReportedState(iotHubClientHandle, reportedState, size, reportedStateCallback, userContextCallback)
#define PnpBridgeClient_SendEventAsync(iotHubClientHandle, eventMessageHandle, eventConfirmationCallback, userContextCallback) IoTHubModuleClient_LL_SendEventAsync(iotHubClientHandle, eventMessageHandle, eventConfirmationCallback, userContextCallback)
#else
#define CLIENT_LL_HANDLE IOTHUB_DEVICE_CLIENT_LL_HANDLE
#define PnpBridgeClient_SendReportedState(iotHubClientHandle, reportedState, size, reportedStateCallback, userContextCallback) IoTHubDeviceClient_LL_SendReportedState(iotHubClientHandle, reportedState, size, reportedStateCallback, userContextCallback)
#define PnpBridgeClient_SendEventAsync(iotHubClientHandle, eventMessageHandle, eventConfirmationCallback, userContextCallback) IoTHubDeviceClient_LL_SendEventAsync(iotHubClientHandle, eventMessageHandle, eventConfirmationCallback, userContextCallback)
#endif

#define PNP_BRIDGE_CLIENT_HANDLE CLIENT_LL_HANDLE


typedef struct _IMPINJ_R700_FIRMWARE_VERSION
{
    long major;
    long minor;
    long build_major;
    long build_minor;
} IMPINJ_R700_FIRMWARE_VERSION, *PIMPINJ_R700_FIRMWARE_VERSION;

#define R700_REST_VERSION_VALUES \
    V_Unknown,                   \
        V1_0,                    \
        V1_2,                    \
        V1_3,                    \
        V1_4

MU_DEFINE_ENUM_WITHOUT_INVALID(R700_REST_VERSION, R700_REST_VERSION_VALUES);

typedef struct _IMPINJ_R700_API_VERSION
{
    IMPINJ_R700_FIRMWARE_VERSION Firmware;
    R700_REST_VERSION RestVersion;

} IMPINJ_R700_API_VERSION, *PIMPINJ_R700_API_VERSION;

static IMPINJ_R700_API_VERSION IMPINJ_R700_API_MAPPING[] = {
    {7, 1, 0, 0, V1_0},
    {7, 3, 0, 0, V1_2},
    {7, 4, 0, 0, V1_3},
    {7, 5, 0, 0, V1_4},
};

typedef struct IMPINJ_READER_STATE_TAG
{
    char* componentName;
    char* customerName;
    int statusHeartbeatSec;
    char* state;
    char* preset;

} IMPINJ_READER_STATE, *PIMPINJ_READER_STATE;

typedef struct _IMPINJ_READER
{
    R700_REST_VERSION ApiVersion;
    THREAD_HANDLE WorkerHandle;
    volatile bool ShuttingDown;
    PIMPINJ_READER_STATE SensorState;
    CLIENT_LL_HANDLE ClientHandle;
    const char* ComponentName;
    CURL_Static_Session_Data* curl_polling_session;
    CURL_Static_Session_Data* curl_static_session;
    CURL_Stream_Session_Data* curl_stream_session;
} IMPINJ_READER, *PIMPINJ_READER;

//
// Application state associated with the particular component. In particular it contains
// the resources used for responses in callbacks along with properties set
// and representations of the property update and command callbacks invoked on given component
//

typedef struct
{
    void* deviceData;
    bool isConnected;
    CLIENT_LL_HANDLE deviceClient;
    char* iothub_uri;
    bool isProvisioned;
} APP_CONTEXT, *PAPP_CONTEXT;

// Map IoT Plug and Play Bridge types to generic SDK types
typedef APP_CONTEXT* PNPBRIDGE_COMPONENT_HANDLE;
#define PnpComponentHandleGetContext(AppContext) (AppContext->deviceData)
#define PnpComponentHandleGetClientHandle(AppContext) (AppContext->deviceClient)

void deviceTwin_CB(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char* payload, size_t size, void* userContextCallback);
int deviceMethod_CB(const char* methodName, const unsigned char* payload, size_t size, unsigned char** response, size_t* responseSize, void* userContextCallback);
static bool BuildUtcTimeFromCurrentTime(char* utcTimeBuffer, size_t utcTimeBufferSize);


#ifdef __cplusplus
}
#endif

#endif /* R700_READER_H */