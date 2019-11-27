#!/bin/bash -xv
# E.g. ./pushwin01.sh -p ../external/EasyProspectServiceDeveloper.0.0.1.nupkg -k ../../../nuget.txt

nppath=""
vtest=0

NUGET=nuget
KEYFILE="key.txt"
while getopts "p:k:t" OPTION; do
  case ${OPTION} in
    p ) nppath="$OPTARG"
      ;;
	k ) KEYFILE="$OPTARG"
      ;;
    t ) vtest=1
      ;;
    \? ) echo "Usage: cmd [-p] path [-t]"
      ;;
  esac
done

#if [[ $vtest -eq 1 && -d "$vpath" ]]; then
#  exit 0
#fi

if [[ -z "$nppath" ]]; then
  exit 1
fi

if [[ ! -e "$KEYFILE" ]]; then
  echo "ERROR: ${KEYFILE} does not exist."
  exit 1
fi

APIKEY=$(<$KEYFILE)

if [[ -z "$APIKEY" ]]; then
  echo "ERROR: API Key is blank."
  exit 1
fi

$NUGET setApiKey ${APIKEY} -Source http://ci01.propsystem.com:8081/repository/easyprospect-service-developer/
$NUGET push "${nppath}" -Source http://ci01.propsystem.com:8081/repository/easyprospect-service-developer/