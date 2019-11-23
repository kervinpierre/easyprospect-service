#!/bin/bash -xv

vpath=""
vtest=0

if [[ -n "${DEPS_DIR}" ]]; then
  vpath="${DEPS_DIR}/vcpkg"
fi

vpath="${DEPS_DIR}/vcpkg"

while getopts "p:t" OPTION; do
  case ${OPTION} in
    p ) vpath="$OPTARG"
      ;;
    t ) vtest=1
      ;;
    \? ) echo "Usage: cmd [-p] path [-t]"
      ;;
  esac
done

if [[ $vtest -eq 1 && -d "$vpath" ]]; then
  exit 0
fi

if [[ -z "$vpath" ]]; then
  exit 1
fi

mkdir -p "$vpath"
pushd "$vpath"
git init
git remote add origin https://github.com/Microsoft/vcpkg.git
git fetch origin master
git checkout -b master origin/master
./bootstrap-vcpkg.sh
popd
