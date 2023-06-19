set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra)

add_executable(kv-builder kv-builder.cpp)

message(STATUS "The value of my_variable is: ${Boost_INCLUDE_DIRS} ${Boost_LIBRARIES}")


# target_include_directories(kv-builder PRIVAT)

target_link_libraries(kv-builder ${Boost_LIBRARIES})

add_executable(kv-loader kv-loader.cpp)
target_link_libraries(kv-loader ${Boost_LIBRARIES})