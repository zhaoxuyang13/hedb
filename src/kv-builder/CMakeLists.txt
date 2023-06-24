set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra)

add_executable(kv-builder kv-builder.cpp)
target_link_libraries(kv-builder ${Boost_LIBRARIES})

# target_include_directories(kv-builder PRIVAT)



add_executable(kv-loader 
        kv-loader.cpp 
        ../extension/kv_ops/kv.cpp
        )
target_link_libraries(kv-loader ${Boost_LIBRARIES})


add_executable(kv-test shm-example.cpp)
target_link_libraries(kv-test ${Boost_LIBRARIES})