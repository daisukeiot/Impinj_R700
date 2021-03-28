// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifndef R700_UTILS_H
#define R700_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <parson.h>
#include "azure_c_shared_utility/xlogging.h"
#include "impinj_reader_r700.h"

static const char g_separator[] = "\r\n================================\r\n";

void LogJsonPretty(
    const char* MsgFormat,
    JSON_Value* JsonValue, ...);

void LogJsonPrettyStr(
    const char* MsgFormat,
    char* JsonString, ...);

JSON_Value*
Remove_JSON_Array(
    char* Payload,
    const char* ArrayName);

JSON_Value*
JSONArray2DtdlMap(
    char* Payload,
    const char* ArrayName,
    const char* MapName);

char* DtdlMap2JSONArray(
    const char* Payload,
    const char* ArrayName);

const char*
GetStringFromPayload(
    JSON_Value* CommandValue,
    const char* ParamName);

const char*
GetObjectStringFromPayload(
    JSON_Value* Payload,
    const char* ParamName);

#ifndef PNPBRIDGE
// Format used when building a response for a root property that does not contain metadata
static const char g_propertyWithoutResponseSchemaWithoutComponent[] = "{\"%s\":%s}";
static const char g_propertyWithoutResponseSchemaWithComponent[] = "{\"""%s\":{\"__t\":\"c\",\"%s\":%s}}";
static const char g_propertyWithResponseSchemaWithoutComponent[] =  "{\"%s\":{\"value\":%s,\"ac\":%d,\"ad\":\"%s\",\"av\":%d}}";
static const char g_propertyWithResponseSchemaWithComponent[] =  "{\"""%s\":{\"__t\":\"c\",\"%s\":{\"value\":%s,\"ac\":%d,\"ad\":\"%s\",\"av\":%d}}}";
static const char PnP_TelemetryComponentProperty[] = "$.sub";

STRING_HANDLE PnP_CreateReportedProperty(const char *componentName, const char *propertyName, const char *propertyValue);
STRING_HANDLE PnP_CreateReportedPropertyWithStatus(const char *componentName, const char *propertyName, const char *propertyValue, int result, const char *description, int ackVersion);
IOTHUB_MESSAGE_HANDLE PnP_CreateTelemetryMessageHandle(const char* componentName, const char* telemetryData);

#endif

#ifdef __cplusplus
}
#endif

#endif /* R700_UTILS_H */