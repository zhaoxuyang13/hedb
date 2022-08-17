
file(GLOB TEST_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    "*.cpp"
)
# file(GLOB INTERFACE_SRC )
message(STATUS "test_src has : ${TEST_SRC}")

foreach(testsource ${TEST_SRC})
    message(STATUS "processing ${testsource}")
    string(REPLACE ".cpp" "" testname ${testsource})
    add_executable(${testname} ${testsource} ../interface/interface.cpp)
    add_dependencies(${testname} ${interface} )
    target_link_libraries(${testname} ${interface})
    set_target_properties(${testname} PROPERTIES 
                            RUNTIME_OUTPUT_DIRECTORY
                            ${CMAKE_CURRENT_SOURCE_DIR})      
endforeach(testsource)


