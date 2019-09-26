#!/usr/bin/env bash
set -eo pipefail

printf "\t=========== Building eosdao.contracts ===========\n\n"
RED='\033[0;31m'
NC='\033[0m'

# set to installed dir
export eosio_DIR=~/eosio/1.8

CPU_CORES=$(getconf _NPROCESSORS_ONLN)
mkdir -p build
pushd build &> /dev/null
cmake ../
make -j $CPU_CORES
printf "\t=========== Running tests ===========\n\n"
./tests/unit_test -- --verbose
popd &> /dev/null
