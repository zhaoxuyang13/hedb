find_package(PostgreSQL REQUIRED)

set(CMAKE_CXX_STANDARD 14)
include_directories(${PostgreSQL_INCLUDE_DIRS}
                    ${PostgreSQL_SERVER_INCLUDE_DIRS}
                    include
                    ../include
                    )
# include(CTest)

file(GLOB SRC
  "interface/*.cpp"
  "*.cpp"
  "../utils/*.c")

add_postgresql_extension(
    encdb
    VERSION 0.0.1
    SOURCES ${SRC}
    SCRIPTS encdb--0.0.1.sql
    REGRESS basic
  )

# extension depend on enclave interface to be build first. (enclave_u.* files)
if ( ${TEE_TYPE} STREQUAL "SGX" )
  add_dependencies(encdb SGXInterface)
  target_link_libraries(encdb SGXInterface)
elseif ( ${TEE_TYPE} STREQUAL "TZ" )
  add_dependencies(encdb TZInterface)
  target_link_libraries(encdb TZInterface)
endif()