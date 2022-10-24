
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

    # set(_link_flags "${PostgreSQL_SHARED_LINK_OPTIONS}")
    # foreach(_dir ${PostgreSQL_SERVER_LIBRARY_DIRS})
    #   set(_link_flags "${_link_flags} -L${_dir}")
    # endforeach()


    set_target_properties(${testname} PROPERTIES 
                            RUNTIME_OUTPUT_DIRECTORY
                            ${CMAKE_CURRENT_SOURCE_DIR})      
endforeach(testsource)


