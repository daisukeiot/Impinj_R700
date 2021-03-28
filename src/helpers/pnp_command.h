
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifndef R700_COMMAND_H
#define R700_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <parson.h>
#ifdef PNPBRIDGE
#include <pnpadapter_api.h>
#endif
#include "pnp_utils.h"

static const char g_powerSourceAlreadyConfigure[] = "{\"message\":\"The provided power source was already configured on the reader.\"}";
static const char g_noContentResponse[]           = "{\"message\":\"Operation Completed\"}";

int OnCommandCallback(
    PNPBRIDGE_COMPONENT_HANDLE PnpComponentHandle,
    const char* CommandName,
    JSON_Value* CommandValue,
    unsigned char** CommandResponse,
    size_t* CommandResponseSize);

char* PreProcessSetPresetIdPayload(
    JSON_Value* Payload);

char* PreProcessTagPresenceResponse(
    JSON_Value* Payload);

#ifdef __cplusplus
}
#endif

#endif /* R700_COMMAND_H */