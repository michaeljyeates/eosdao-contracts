add_contract(token token ${CMAKE_CURRENT_SOURCE_DIR}/src/token.cpp)

target_include_directories(token
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/token.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/token.contracts.md @ONLY )

target_compile_options( token PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
