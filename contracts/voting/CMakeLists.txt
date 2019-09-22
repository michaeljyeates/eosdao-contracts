add_contract(voting voting ${CMAKE_CURRENT_SOURCE_DIR}/src/voting.cpp)
find_path(LIBEOSDAC_INCLUDE_DIR NAMES libeosdac/libeosdac.hpp PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../external NO_CMAKE_FIND_ROOT_PATH)
message(STATUS "libeosdac => ${LIBEOSDAC_INCLUDE_DIR}")

if((NOT LIBEOSDAC_INCLUDE_DIR) OR (NOT EXISTS ${LIBEOSDAC_INCLUDE_DIR}))
    message("Unable to find libeosdac ${LIBEOSDAC_INCLUDE_DIR} in voting contract.  Please make sure you have run git submodule update --init)")
endif()

target_include_directories(voting
   PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBEOSDAC_INCLUDE_DIR})

set_target_properties(voting
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/voting.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/voting.contracts.md @ONLY )

target_compile_options( voting PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
