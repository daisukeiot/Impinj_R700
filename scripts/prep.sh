#
# Script to : 
#  - Clone Azure IoT SDK C
#  - Generate self-signed X.509 certificate and device key (./cmake/new-device.cert.pem and ./cmake/new-device.key.pem)
#  - Compile sample device application for dtmi:com:Example:Thermostat;1
#
# Daisuke Nakahara (daisuken@microsoft.com)
#

print_usage() {
    # HELP_TEXT="Usage:\n" "-f : Force to create new X509 certificates\n" \
    #      "-c : Clean up.  Removes certificates and Azure IoT SDK C\n" \
    #      "-v : Verbose\n" \
    #      "-h : This help menu\n"
    printf '%s\n' "Usage: $0 [-f] [-v] [-h]" \
                  '  -f : Force to create new X509 certificates' \
                  '  -v : Verbose' \
                  '  -h : This help menu'
}

Log() {
    if [ $VERBOSE = true ]; then
        printf "%s\n" "$1"
    fi
}

SCRIPT=`realpath -s $0`
SCRIPTPATH=`dirname $SCRIPT`
ROOTPATH="$(dirname "$SCRIPTPATH")"
FOLDER_SDK=${ROOTPATH}/azure-iot-sdk-c
FOLDER_CMAKE=${ROOTPATH}/cmake
FORCE=false
VERBOSE=false
CLEAN=false

while getopts 'cfvh' flag; do
  case "${flag}" in
    c) CLEAN=true ;;
    f) FORCE=true ;;
    v) VERBOSE=true ;;
    h) print_usage
       exit 1 ;;
    *) print_usage
       exit 1 ;;
  esac
done

echo "Force   $FORCE"
echo "Clean   $CLEAN"
echo "Verbose $VERBOSE"

if [ $CLEAN = true ]; then
    for folder in "${FOLDER_SDK}" "${FOLDER_CMAKE}"
    do
        if [ -d "$folder" ]; then
            Log "Removing $folder"
            rm -r -f "$folder"
        fi
    done
fi

cd $ROOTPATH

# Clone Azure IoT SDK C
if [ ! -d "${FOLDER_SDK}" ]; then
    if [ $VERBOSE = true ]; then
        git clone https://github.com/Azure/azure-iot-sdk-c --recursive
    else
        printf "%s\n" "Cloning Azure IoT SDK C to ${FOLDER_SDK}"
        git clone https://github.com/Azure/azure-iot-sdk-c --recursive --quiet
    fi    
fi

if [ ! -d "${FOLDER_SDK}" ]; then
    printf "ERROR : %s\n" "Azure IoT SDK C not found"
    exit 1;
else
    printf "%s\n" "Azure IoT SDK C found"
fi

# Compile the code
if [ ! -d "${FOLDER_CMAKE}" ]; then
    mkdir "${FOLDER_CMAKE}"
fi

cd "${FOLDER_CMAKE}"
cmake .. -Duse_prov_client=ON -Dhsm_type_x509:BOOL=OFF -Dhsm_type_symm_key:BOOL=ON -Dskip_samples:BOOL=OFF -Duse_amqp:BOOL=OFF -Dbuild_service_client:BOOL=OFF -Duse_http=:BOOL=OFF -Duse_amqp=:BOOL=OFF -Dbuild_provisioning_service_client=:BOOL=OFF -Drun_e2e_tests=OFF
cmake --build .
