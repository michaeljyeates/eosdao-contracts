#!/usr/bin/env bash
set -eo pipefail

CPU_CORES=$(getconf _NPROCESSORS_ONLN)
GREEN='\033[0;32m'
NC='\033[0m'

printf "\t${GREEN}=========== Building eosdao contracts with ${CPU_CORES} cores ===========${NC}\n\n"

mkdir -p build
pushd build &> /dev/null
cmake ../
make -j $CPU_CORES
popd &> /dev/null
