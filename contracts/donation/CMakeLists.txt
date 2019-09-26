add_contract(donation donation ${CMAKE_CURRENT_SOURCE_DIR}/src/donation.cpp)
find_path(LIBEOSDAC_INCLUDE_DIR NAMES libeosdac/libeosdac.hpp PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../external NO_CMAKE_FIND_ROOT_PATH)

if((NOT LIBEOSDAC_INCLUDE_DIR) OR (NOT EXISTS ${LIBEOSDAC_INCLUDE_DIR}))
    message("Unable to find libeosdac ${LIBEOSDAC_INCLUDE_DIR}")
endif()

target_include_directories(donation
   PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBEOSDAC_INCLUDE_DIR})

set_target_properties(donation
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/donation.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/donation.contracts.md @ONLY )

target_compile_options( donation PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
