set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS system filesystem serialization REQUIRED)

file(GLOB SRC kv-builder.cpp)

add_compile_options(-Wall -Wextra)

add_executable(kv-builder ${SRC})


message(STATUS "The value of my_variable is: ${Boost_INCLUDE_DIRS} ${Boost_LIBRARIES}")


target_include_directories(kv-builder PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(kv-builder ${Boost_LIBRARIES})
