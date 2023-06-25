cmake_minimum_required(VERSION 3.9)
project(extension)

find_package(Boost COMPONENTS system filesystem serialization REQUIRED)

include_directories(include 
                    ${Boost_INCLUDE_DIRS})

add_subdirectory(kv-builder)
add_subdirectory(extension)
