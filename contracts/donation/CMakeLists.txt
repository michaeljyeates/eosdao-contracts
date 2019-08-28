add_contract(donation donation ${CMAKE_CURRENT_SOURCE_DIR}/src/donation.cpp)

target_include_directories(donation
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(donation
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/donation.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/donation.contracts.md @ONLY )

target_compile_options( donation PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
