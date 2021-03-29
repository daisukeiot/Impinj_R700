// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Sample IoT Plug and Play device app for X.509 certificate attestation
//
// Daisuke Nakahara
//

#include "deviceTwin.h"

/* 
** Callback function to receive Device Twin update notification
*/
void deviceTwinCallback(
    DEVICE_TWIN_UPDATE_STATE updateState,
    const unsigned char* payload,
    size_t size,
    void* userContextCallback)
{
    CLIENT_LL_HANDLE deviceClient = (CLIENT_LL_HANDLE)userContextCallback;
    APP_CONTEXT* appContext       = (APP_CONTEXT*)userContextCallback;
    char* paylod_copy             = (char*)calloc(1, size + 1);
    JSON_Value* jsonVal_Payload;
    char* pretty;

    if (paylod_copy == NULL)
    {
        LogError("failed to allocate json buffer");
    }
    else
    {
        (void)memcpy(paylod_copy, payload, size);

        jsonVal_Payload = json_parse_string(paylod_copy);

        if (jsonVal_Payload == NULL)
        {
            LogError("filed to parse device twin payload");
        }
        else
        {
            pretty = json_serialize_to_string_pretty(jsonVal_Payload);

            if (pretty)
            {
                LogInfo("%s() : Update Status %s Paload \r\n%s", __func__, MU_ENUM_TO_STRING(DEVICE_TWIN_UPDATE_STATE, updateState), pretty);
                json_free_serialized_string(pretty);
            }

            if (_deviceTwinCallback_fn != NULL)
            {
                _deviceTwinCallback_fn(updateState, payload, size, userContextCallback);
            }

            json_value_free(jsonVal_Payload);
        }

        free(paylod_copy);
    }
}

static void ReportStatusCallback(int result, void* context)
{
    LogInfo("Device Twin reported properties update result: Status code %d", result);
}
