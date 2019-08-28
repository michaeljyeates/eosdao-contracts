
cmake_minimum_required(VERSION 3.5)
project(daccustodian VERSION 1.0.0)

find_package(eosio.cdt)

### Generate the wasm and abi
add_contract( daccustodian daccustodian
        daccustodian.cpp
        external_observable_actions.cpp
        newperiod_components.cpp
        pay_handling.cpp
        privatehelpers.cpp
        registering.cpp
        update_member_details.cpp
        )

set_target_properties(daccustodian
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

### add the path to where the ricardian contracts/clauses are found
target_compile_options( daccustodian PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR} )
