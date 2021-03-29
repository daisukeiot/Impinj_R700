#////////////////////////////////////////////////////////////////////////////
#//
#// CMake to build Simple Thermostat Demo
#//
#////////////////////////////////////////////////////////////////////////////

#////////////////////////////////////////////////////////////////////////////
#// Project setup
#////////////////////////////////////////////////////////////////////////////
cmake_minimum_required(VERSION 2.8.11)
project(impinj_r700)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")

#Compile As C99
if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
    if (CXX_FLAG_CXX11)
      set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
    else()
      set (CMAKE_CXX_FLAGS "--std=c++0x ${CMAKE_CXX_FLAGS}")
    endif()
  endif()
else()
  set (CMAKE_C_STANDARD 99)
  set (CMAKE_CXX_STANDARD 11)
endif()

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

#////////////////////////////////////////////////////////////////////////////
#// Build Type
#////////////////////////////////////////////////////////////////////////////
if(NOT CMAKE_CONFIGURATION_TYPES)
	if(DEFINED CMAKE_BUILD_TYPE)
		message("-- Set: " ${CMAKE_BUILD_TYPE})
		set(CMAKE_CONFIGURATION_TYPES Debug)
	else()
		message("-- Set: " Release)
		set(CMAKE_CONFIGURATION_TYPES Release)
	endif()
endif()

message("-- Configuration types: " ${CMAKE_CONFIGURATION_TYPES})

if(CMAKE_CONFIGURATION_TYPES MATCHES Debug)
    ADD_DEFINITIONS(-DDEBUG)
endif()

set(pnp-app_h_files
    ./src/iothub/dps.h
    ./src/iothub/callback.h
    ./src/iothub/deviceMethod.h
    ./src/iothub/deviceTwin.h
    ./src/iothub/iothub_op.h
    ./src/curl_wrapper/curl_wrapper.h
    ./src/helpers/pnp_utils.h
    ./src/helpers/restapi.h
    ./src/helpers/pnp_property.h
    ./src/helpers/pnp_command.h
    ./src/helpers/pnp_telemetry.h
    ./src/helpers/led.h
    ./src/impinj_reader_r700.h
)

set(pnp-app_c_files
    ./src/iothub/dps.c
    ./src/iothub/callback.c
    ./src/iothub/deviceMethod.c
    ./src/iothub/deviceTwin.c
    ./src/iothub/iothub_op.c
    ./src/curl_wrapper/curl_wrapper.c
    ./src/helpers/pnp_utils.c
    ./src/helpers/restapi.c
    ./src/helpers/pnp_property.c
    ./src/helpers/pnp_command.c
    ./src/helpers/pnp_telemetry.c
    ./src/helpers/led.c
    ./src/main.c
)

add_subdirectory(./azure-iot-sdk-c)
include_directories(./src)
include_directories(${azure_iot_sdks_SOURCE_DIR}/deps/parson)
include_directories(${azure_iot_sdks_SOURCE_DIR}/deps/parson)
include_directories(${MQTT_INC_FOLDER})
include_directories(${SHARED_UTIL_INC_FOLDER})
include_directories(${UMOCK_C_INC_FOLDER})
include_directories(${MACRO_UTILS_INC_FOLDER})
include_directories(${IOTHUB_CLIENT_INC_FOLDER})
include_directories(${DEV_AUTH_MODULES_CLIENT_INC_FOLDER})

add_executable(impinj_r700 ${pnp-app_c_files} ${pnp-app_h_files})

target_link_libraries(impinj_r700 
                      parson
                      prov_mqtt_transport
                      iothub_client
                      iothub_client_mqtt_transport
                      prov_device_ll_client
                      prov_mqtt_transport
                      prov_auth_client
                      aziotsharedutil
                      umqtt
                      pthread
                      curl
                      )
