add_contract(voting voting ${CMAKE_CURRENT_SOURCE_DIR}/src/voting.cpp)

target_include_directories(voting
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(voting
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/voting.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/voting.contracts.md @ONLY )

target_compile_options( voting PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
