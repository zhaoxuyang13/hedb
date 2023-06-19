set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(PostgreSQL REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PostgreSQL_INCLUDE_DIRS}
                    ${PostgreSQL_SERVER_INCLUDE_DIRS}
                    include
                    ../include
                    )
# include(CTest)

file(GLOB SRC
  "*.c"
  "*.cpp"
  "../utils/*.c")

add_postgresql_extension(
    encdb
    VERSION 0.0.1
    SOURCES ${SRC}
    SCRIPTS encdb--0.0.1.sql
  )

target_link_libraries(encdb ${Boost_LIBRARIES})
