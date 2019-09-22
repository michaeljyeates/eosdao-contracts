#!/bin/bash
set -eo pipefail

GREEN='\033[0;32m'
NC='\033[0m'

cd /src
./scripts/build.sh -y

printf "\t${GREEN}=========== Running tests ===========${NC}\n\n"
./build/tests/unit_test
# For debugging
#./build/tests/unit_test -- --verbose
