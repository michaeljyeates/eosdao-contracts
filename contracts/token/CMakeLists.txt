add_contract(token token ${CMAKE_CURRENT_SOURCE_DIR}/src/token.cpp)
find_path(LIBEOSDAC_INCLUDE_DIR NAMES libeosdac/libeosdac.hpp PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../external NO_CMAKE_FIND_ROOT_PATH)

if((NOT LIBEOSDAC_INCLUDE_DIR) OR (NOT EXISTS ${LIBEOSDAC_INCLUDE_DIR}))
    message("Unable to find libeosdac ${LIBEOSDAC_INCLUDE_DIR}")
endif()

target_include_directories(token
   PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBEOSDAC_INCLUDE_DIR})

set_target_properties(token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/token.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/token.contracts.md @ONLY )

target_compile_options( token PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
