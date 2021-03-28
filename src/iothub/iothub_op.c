// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Sample IoT Plug and Play device app for X.509 certificate attestation
//
// Daisuke Nakahara
//

#include "iothub_op.h"
#include "deviceTwin.h"
#include "deviceMethod.h"

#define PNP_ENABLE

static const char* g_pnp_model_id = NULL;

/*
** Receives callback when IoT Hub connection status change
**
*/
static void
connection_status_callback(
    IOTHUB_CLIENT_CONNECTION_STATUS connectionStatus,
    IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason,
    void* userContextCallback)
{
    APP_CONTEXT* appContext = (APP_CONTEXT*)userContextCallback;

    LogInfo("%s() enter", __func__);

    if (connectionStatus == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        LogInfo("Connected to iothub : %s", appContext->iothub_uri);
        appContext->isConnected = true;
    }
    else
    {
        LogError("The device client has been disconnected : Connection Status %s : Reason %s",
                 MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS, connectionStatus),
                 MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason));
        appContext->isConnected = false;
        IoTHub_Deinit();
    }
}

/*
** Opens handle using Connection String
**
** To Do : Add DPS support
*/
static CLIENT_LL_HANDLE
CreateClientHandle(
    APP_CONTEXT* appContext)
{
    const char* iothubCs;
    const char* scopeId;
    const char* deviceId;
    const char* deviceKey;
    const char* x509 = NULL;

    LogInfo("%s() enter", __func__);

    g_pnp_model_id = getenv(IOTPNP_MODEL_ID);

    if ((iothubCs = getenv(IOTHUB_CS)) != NULL)
    {
#ifdef USE_EDGE_MODULES
        return IoTHubModuleClient_LL_CreateFromEnvironment(MQTT_Protocol);
#else
        return IoTHubDeviceClient_LL_CreateFromConnectionString(iothubCs, MQTT_Protocol);
#endif
    }
    else
    {
        if ((scopeId = getenv(DPS_IDSCOPE)) == NULL)
        {
            LogError("Cannot read environment variable=%s", DPS_IDSCOPE);
            return NULL;
        }
#ifndef USE_EDGE_MODULES
        else if ((x509 = getenv(DPS_X509)) != NULL)
        {
            return ProvisionDeviceX509(scopeId, g_pnp_model_id, appContext);
        }
#endif
        else
        {
            if ((deviceId = getenv(DPS_DEVICE_ID)) == NULL)
            {
                LogError("Cannot read environment variable=%s", DPS_DEVICE_ID);
                return NULL;
            }
            else if ((deviceKey = getenv(DPS_SYMMETRIC_KEY)) == NULL)
            {
                LogError("Cannot read environment variable=%s", DPS_SYMMETRIC_KEY);
                return NULL;
            }
            else
            {
                return ProvisionDevice(scopeId, deviceId, deviceKey, g_pnp_model_id, appContext);
            }
        }
    }
    return NULL;
}

/*
** Initialize IoT Hub connection
*/
CLIENT_LL_HANDLE
IoTHubInitialize(
    APP_CONTEXT* appContext)
{
    int iothubInitResult;
    bool isConfigured;
    IOTHUB_CLIENT_RESULT iothubResult;
    CLIENT_LL_HANDLE clientHandle = NULL;
    bool urlAutoEncodeDecode      = true;

    LogInfo("%s() enter", __func__);

    if (appContext == NULL)
    {
        LogError("AppContext is NULL");
    }

    if ((iothubInitResult = IoTHub_Init()) != 0)
    {
        LogError("Failure to initialize client.  Error=%d", iothubInitResult);
        isConfigured = false;
    }
    else if ((clientHandle = CreateClientHandle(appContext)) == NULL)
    {
        LogError("Failure creating IotHub client.  Hint: Check your connection string or DPS configuration");
        isConfigured = false;
    }
#ifdef USE_EDGE_MODULES
    else if ((iothubResult = IoTHubModuleClient_LL_SetOption(clientHandle, OPTION_LOG_TRACE, &g_hubClientTraceEnabled)) != IOTHUB_CLIENT_OK)
#else
    else if ((iothubResult = IoTHubDeviceClient_LL_SetOption(clientHandle, OPTION_LOG_TRACE, &g_hubClientTraceEnabled)) != IOTHUB_CLIENT_OK)
#endif
    {
        LogError("Unable to set logging option, error=%d", iothubResult);
        isConfigured = false;
    }
#ifdef PNP_ENABLE
#ifdef USE_EDGE_MODULES
    else if ((g_pnp_model_id) && ((iothubResult = IoTHubModuleClient_LL_SetOption(clientHandle, OPTION_MODEL_ID, g_pnp_model_id)) != IOTHUB_CLIENT_OK))
#else
    else if ((g_pnp_model_id) && ((iothubResult = IoTHubDeviceClient_LL_SetOption(clientHandle, OPTION_MODEL_ID, g_pnp_model_id)) != IOTHUB_CLIENT_OK))
#endif
    {
        LogError("Unable to set model ID, error=%d", iothubResult);
        isConfigured = false;
    }
#endif
#ifdef USE_EDGE_MODULES
    else if ((iothubResult = IoTHubModuleClient_LL_SetOption(clientHandle, OPTION_AUTO_URL_ENCODE_DECODE, &urlAutoEncodeDecode)) != IOTHUB_CLIENT_OK)
#else
    else if ((iothubResult = IoTHubDeviceClient_LL_SetOption(clientHandle, OPTION_AUTO_URL_ENCODE_DECODE, &urlAutoEncodeDecode)) != IOTHUB_CLIENT_OK)
#endif
    {
        LogError("Unable to set auto Url encode option, error=%d", iothubResult);
        isConfigured = false;
    }
#ifdef USE_EDGE_MODULES
    else if ((iothubResult = IoTHubModuleClient_LL_SetConnectionStatusCallback(clientHandle, connection_status_callback, (void*)appContext)) != IOTHUB_CLIENT_OK)
#else
    else if ((iothubResult = IoTHubDeviceClient_LL_SetConnectionStatusCallback(clientHandle, connection_status_callback, (void*)appContext)) != IOTHUB_CLIENT_OK)
#endif
    {
        LogError("Unable to set Connection Status callback, error=%d", iothubResult);
        isConfigured = false;
    }
#ifdef USE_EDGE_MODULES
    else if ((iothubResult = IoTHubModuleClient_LL_SetModuleMethodCallback(clientHandle, deviceMethodCallback, (void*)appContext)) != IOTHUB_CLIENT_OK)
#else
    else if ((iothubResult = IoTHubDeviceClient_LL_SetDeviceMethodCallback(clientHandle, deviceMethodCallback, (void*)appContext)) != IOTHUB_CLIENT_OK)
#endif
    {
        LogError("Unable to set device method callback, error=%d", iothubResult);
        isConfigured = false;
    }
#ifdef USE_EDGE_MODULES
    else if ((iothubResult = IoTHubModuleClient_LL_SetModuleTwinCallback(clientHandle, deviceTwinCallback, (void*)appContext)) != IOTHUB_CLIENT_OK)
#else
    else if ((iothubResult = IoTHubDeviceClient_LL_SetDeviceTwinCallback(clientHandle, deviceTwinCallback, (void*)appContext)) != IOTHUB_CLIENT_OK)
#endif
    {
        LogError("Unable to set Device Twin callback, error=%d", iothubResult);
        isConfigured = false;
    }
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
    // Setting the Trusted Certificate.  This is only necessary on systems without built in certificate stores.
#ifdef USE_EDGE_MODULES
    else if ((iothubResult = IoTHubModuleClient_LL_SetOption(clientHandle, deviceTwinCallback, (void*)appContext)) != IOTHUB_CLIENT_OK)
#else
    else if ((iothubResult = IoTHubDeviceClient_LL_SetOption(clientHandle, OPTION_TRUSTED_CERT, certificates)) != IOTHUB_CLIENT_OK)
#endif
    {
        LogError("Unable to set the trusted cert, error=%d", iothubResult);
        isConfigured = false;
    }
#endif   // SET_TRUSTED_CERT_IN_SAMPLES
    else
    {
        isConfigured = true;
    }

    if ((isConfigured == false) && (clientHandle != NULL))
    {
#ifdef USE_EDGE_MODULES
        IoTHubModuleClient_LL_Destroy(clientHandle);
#else
        IoTHubDeviceClient_LL_Destroy(clientHandle);
#endif
        clientHandle = NULL;
    }

    if ((isConfigured == false) && (iothubInitResult == 0))
    {
        IoTHub_Deinit();
    }

    return clientHandle;
}