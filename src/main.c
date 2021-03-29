// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Sample IoT Plug and Play reader app for X.509 certificate attestation
//
// Daisuke Nakahara
//
#include "impinj_reader_r700.h"
#include "iothub/callback.h"
#include "iothub/iothub_op.h"
#include "helpers/pnp_property.h"
#include "helpers/pnp_telemetry.h"
#include "helpers/pnp_command.h"

char g_ProgramStartTime[128];
static const char R700_HOSTNAME[] = "ReaderHost";
static const char R700_USERNAME[] = "ReaderUser";
static const char R700_PASSWORD[] = "ReaderPass";

MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(R700_REST_VERSION, R700_REST_VERSION_VALUES);


void UninitializeImpinjReader(
    PIMPINJ_READER Reader)
{
    int res;

    if (Reader)
    {
        Reader->ShuttingDown = true;

        if (Reader->curl_stream_session)
        {
            curlStreamStopThread(Reader->curl_stream_session);

            if (ThreadAPI_Join(Reader->WorkerHandle, &res) != THREADAPI_OK)
            {
                LogError("ThreadAPI_Join failed");
            }

            curlStreamCleanup(Reader->curl_stream_session);
        }

        if (Reader->curl_polling_session)
        {
            curlStaticCleanup(Reader->curl_polling_session);
        }

        if (Reader->curl_static_session)
        {
            curlStaticCleanup(Reader->curl_static_session);
        }

        if (Reader->SensorState != NULL)
        {
            free(Reader->SensorState);
        }

        free(Reader);

        curl_global_cleanup();
    }
}

bool InitilizeImpinjReader(
    APP_CONTEXT* AppContext)
{
    bool result         = true;
    char* http_hostname = NULL;
    char* http_user     = NULL;
    char* http_pass     = NULL;
    char* http_basepath = NULL;
    PIMPINJ_READER reader;

    char http_base_url[128] = {0};

    AppContext->deviceData = NULL;

    if ((http_hostname = getenv(R700_HOSTNAME)) == NULL)
    {
        LogError("Reader Host name missing");
    }
    else if ((http_user = getenv(R700_USERNAME)) == NULL)
    {
        LogError("Reader user name missing");
        result = false;
    }
    else if ((http_pass = getenv(R700_PASSWORD)) == NULL)
    {
        LogError("Reader password missing");
    }
    else
    {
        if ((reader = calloc(1, sizeof(IMPINJ_READER))) == NULL)
        {
            LogError("Unable to allocate memory for Impinj Reader.");
        }
        else if ((reader->SensorState = calloc(1, sizeof(IMPINJ_READER_STATE))) == NULL)
        {
            LogError("Unable to allocate memory for Impinj Reader state.");
        }
        else
        {
            LogInfo("R700 :  Reader Hostname=%s User=%s Password=%s", http_hostname, http_user, http_pass);
            curl_global_init(CURL_GLOBAL_DEFAULT);

            sprintf(http_base_url, "https://%s/api/v1", http_hostname);

            mallocAndStrcpy_s(&http_basepath, http_base_url);

            if ((reader->curl_polling_session = curlStaticInit(http_user, http_pass, http_basepath, VERIFY_CERTS_OFF, VERBOSE_OUTPUT_OFF)) == NULL)
            {
                LogError("Unable to allocate CURL static session for polling");
            }
            else if ((reader->curl_static_session = curlStaticInit(http_user, http_pass, http_basepath, VERIFY_CERTS_OFF, VERBOSE_OUTPUT_OFF)) == NULL)
            {
                LogError("Unable to allocate CURL static session for REST API");
            }
            else if ((reader->curl_stream_session = curlStreamInit(http_user, http_pass, http_basepath, VERIFY_CERTS_OFF, VERBOSE_OUTPUT_OFF)) == NULL)
            {
                LogError("Unable to allocate CURL session for HTTP Stream");
            }
            else if (curlStreamSpawnReaderThread(reader->curl_stream_session) != THREADAPI_OK)
            {
                LogError("Unable to create Stream Reader thread");
            }
            // Create a thread to periodically publish telemetry
            else
            {
                GetFirmwareVersion(reader);
            }

            AppContext->deviceData = reader;

            if (ThreadAPI_Create(&reader->WorkerHandle, ImpinjReader_TelemetryWorker, AppContext) != THREADAPI_OK)
            {
                LogError("Unabled to create Telemetry worker thread");
            }
        }
    }
exit:

    return AppContext->deviceData
               ? true
               : false;
}

int main(int argc, char* argv[])
{
    CLIENT_LL_HANDLE clientHandle = NULL;
    char msgBuffer[1024];
    APP_CONTEXT* appContext;

    printf("Starting Simple Thermostat IoT Hub Device App\r\n");

    appContext = calloc(1, sizeof(APP_CONTEXT));

    if (appContext == NULL)
    {
        printf("Failed to allocate app context\r\n");
        return -1;
    }

    if (!InitilizeImpinjReader(appContext))
    {
        LogError("Unable to initialize Impinj reader data");
        UninitializeImpinjReader(appContext->deviceData);
    }
    else
    {

        BuildUtcTimeFromCurrentTime(g_ProgramStartTime, sizeof(g_ProgramStartTime));
        appContext->isConnected = false;

        while (true)
        {
            if (appContext->isConnected != true)
            {
                if (clientHandle == NULL)
                {
                    if ((appContext->deviceClient = IoTHubInitialize(appContext)) == NULL)
                    {
                        LogError("Failed to initialize IoT Hub connection.  Retrying in 5 seconds");
                        ThreadAPI_Sleep(5000);
                        continue;
                    }
                    else
                    {
                        set_callback(CB_DEVICE_TWIN, deviceTwin_CB, appContext);
                        set_callback(CB_DEVICE_METHOD, deviceMethod_CB, appContext);
                    }
                }

                // wait for connection up to 10 sec
                int retry = 100;
                while (appContext->isConnected != true && retry > 0)
                {
                    ThreadAPI_Sleep(100);
#ifdef USE_EDGE_MODULES
                    IoTHubModuleClient_LL_DoWork(appContext->deviceClient);
#else
                    IoTHubDeviceClient_LL_DoWork(appContext->deviceClient);
#endif
                }

                if (appContext->isConnected != true)
                {
                    LogError("IoT Hub connection not established after 10 seconds");
                }
            }

            if (appContext->isConnected == true)
            {
                // process periodic tasks
                // e.g. sending telemetry
                // Telemetry is processed in a separate thread
                ThreadAPI_Sleep(1000);
            }
            else
            {
                LogInfo("R700 :  Not Connected");
            }

#ifdef USE_EDGE_MODULES
            IoTHubModuleClient_LL_DoWork(appContext->deviceClient);
#else
            IoTHubDeviceClient_LL_DoWork(appContext->deviceClient);
#endif
        }
    }

    if (appContext != NULL)
    {
        UninitializeImpinjReader(appContext->deviceData);
        free(appContext);
    }

    return 0;
}

/*
** Callback on Device Twin changes
*/
void deviceTwin_CB(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char* payload, size_t size, void* userContextCallback)
{
    APP_CONTEXT* appContext   = (APP_CONTEXT*)userContextCallback;
    JSON_Value* root_Value    = NULL;
    JSON_Object* root_Object  = NULL;
    JSON_Value* version_Value = NULL;
    int version;
    size_t i;
    IOTHUB_CLIENT_RESULT result;

    if ((root_Value = json_parse_string(payload)) == NULL)
    {
        LogError("Unable to parse Device Twin payload");
    }
    else if ((root_Object = json_value_get_object(root_Value)) == NULL)
    {
        LogError("Unable to get root object of JSON");
    }
    else
    {
        if (updateState == DEVICE_TWIN_UPDATE_COMPLETE)
        {
            OnPropertyCompleteCallback(appContext, root_Value, NULL);
        }
        else
        {
            if (root_Object == NULL)
            {
                LogError("Could not find Desired Property");
            }
            else if ((version_Value = json_object_get_value(root_Object, "$version")) == NULL)
            {
                LogError("Could not find $version");
            }
            else if (json_value_get_type(version_Value) != JSONNumber)
            {
                LogError("$version is not a number");
            }
            else
            {
                int node_count = json_object_get_count(root_Object);
                version        = json_value_get_number(version_Value);

                for (i = 0; i < node_count; i++)
                {
                    const char* node_Name = json_object_get_name(root_Object, i);

                    if (strcmp(node_Name, "$version") == 0)
                    {
                        continue;
                    }
                    JSON_Value* desired_Value = json_object_get_value_at(root_Object, i);

                    OnPropertyPatchCallback(appContext, node_Name, desired_Value, version, NULL);
                }
            }
        }
    }
}

/*
** Callback on Device Method
*/
int deviceMethod_CB(const char* methodName, const unsigned char* payload, size_t size, unsigned char** response, size_t* responseSize, void* userContextCallback)
{
    LogInfo("R700 :  %s %s Payload : %s", __func__, methodName, payload);
    APP_CONTEXT* appContext = (APP_CONTEXT*)userContextCallback;
    JSON_Value* rootValue   = NULL;
    int result              = 500;

    if (payload != NULL && (rootValue = json_parse_string(payload)) == NULL)
    {
        LogError("Cannot retrieve JSON string for command");
        result = R700_STATUS_BAD_REQUEST;
    }
    else
    {
        result = OnCommandCallback(appContext, methodName, rootValue, response, responseSize);
    }

    if (rootValue)
    {
        json_value_free(rootValue);
    }

    return result;
}

//
// Build ISO8601 format time stamp string
//
static bool BuildUtcTimeFromCurrentTime(char* utcTimeBuffer, size_t utcTimeBufferSize)
{
    bool result;
    time_t currentTime;
    struct tm* currentTimeTm;
    static const char ISO8601Format[] = "%Y-%m-%dT%H:%M:%SZ";

    time(&currentTime);
    currentTimeTm = gmtime(&currentTime);

    if (strftime(utcTimeBuffer, utcTimeBufferSize, ISO8601Format, currentTimeTm) == 0)
    {
        LogError("snprintf on UTC time failed");
        result = false;
    }
    else
    {
        result = true;
    }

    return result;
}