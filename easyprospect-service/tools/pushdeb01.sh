#!/bin/bash -xv

debpath=""
vtest=0

while getopts "p:t" OPTION; do
  case ${OPTION} in
    p ) debpath="$OPTARG"
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

if [[ -z "$debpath" ]]; then
  exit 1
fi

echo -n "Password:"
apass=""
read -s apass
if [[ -z "$apass" ]]; then
  exit 1
fi

curl -v -u "admin:${apass}" -H "Content-Type: multipart/form-data" --data-binary "@${debpath}" "http://ci01.propsystem.com:8081/repository/easyprospect-repo/"

