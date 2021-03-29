// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Sample IoT Plug and Play device app for X.509 certificate attestation
//
// Daisuke Nakahara
//

#include "deviceMethod.h"

/* 
** Callback function to device Device Method (or Direct Methods)
**
** To Do : Add user callback to process Device Method
*/
int deviceMethodCallback(
    const char* methodName,
    const unsigned char* payload,
    size_t size,
    unsigned char** response,
    size_t* responseSize,
    void* userContextCallback)
{
    CLIENT_LL_HANDLE deviceClient = (CLIENT_LL_HANDLE)userContextCallback;
    APP_CONTEXT* appContext       = (APP_CONTEXT*)userContextCallback;
    char* paylod_copy             = (char*)calloc(1, size + 1);
    JSON_Value* jsonVal_Payload;
    char* pretty;

    const char* RESPONSE_STRING = "{ \"Response\": \"Success\" }";
    int result                  = 200;

    if (paylod_copy == NULL)
    {
        LogError("failed to allocate json buffer");
        result = 500;
    }
    else
    {
        (void)memcpy(paylod_copy, payload, size);

        jsonVal_Payload = json_parse_string(paylod_copy);

        if (jsonVal_Payload == NULL)
        {
            LogError("filed to parse device method payload");
            result = 500;
        }
        else
        {
            pretty = json_serialize_to_string_pretty(jsonVal_Payload);

            if (pretty)
            {
                LogInfo("%s() : Method %s Paload : %s", __func__, methodName, pretty);
                json_free_serialized_string(pretty);
            }

            if (_deviceMethodCallback_fn != NULL)
            {
                result = _deviceMethodCallback_fn(methodName, paylod_copy, size, response, responseSize, userContextCallback);
            }
            else if (*response == NULL)
            {
                *responseSize = strlen(RESPONSE_STRING);

                if ((*response = malloc(*responseSize)) == NULL)
                {
                    result = 500;
                }
                else
                {
                    memcpy(*response, RESPONSE_STRING, *responseSize);
                }
            }

            json_value_free(jsonVal_Payload);
        }

        free(paylod_copy);
    }

    LogInfo("Method : %s return : %d", methodName, result);
    return result;
}
