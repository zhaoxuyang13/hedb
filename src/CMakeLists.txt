cmake_minimum_required(VERSION 3.9)
project(extension)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(SGX REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${SGX_INCLUDE_DIR} include enclave)
set(EDL enclave/sgx/enclave.edl)
set(EDL_SEARCH_PATHS enclave/sgx)

# add_library(SGXInterface STATIC ${U_SRCS})

# file(GLOB T_SRCS LocalAttestationCode/*.c*)
# add_trusted_library(LocalAttestationLib SRCS "${T_SRCS}" EDL LocalAttestationCode/LocalAttestationCode.edl EDL_SEARCH_PATHS ${EDL_SEARCH_PATHS})
file(GLOB E_SRCS enclave/sgx/*.cpp enclave/*.c)
add_enclave_library(enclave
                    USE_PREFIX
                    SRCS ${E_SRCS} 
                    EDL ${EDL} 
                    EDL_SEARCH_PATHS ${EDL_SEARCH_PATHS}
                    LDSCRIPT enclave/sgx/enclave.lds)
## DE
enclave_sign(enclave KEY enclave/sgx/Enclave_private.pem CONFIG enclave/sgx/enclave.config.xml)

file(GLOB U_SRCS extension/interface/sgx/sgx_interface.cpp)
include_directories(extension/include extension/interface/sgx)

# custom installation for enclave.so and datafile.
SET(ENCLAVE_INSTALL_DIR "/usr/local/lib/hedb")
set(SIGNED_ENCLAVE_FILENAME "enclave.signed.so")
SET(DATA_FILENAME "${ENCLAVE_INSTALL_DIR}/hedb.data")
SET(ENCLAVE_FILENAME "${ENCLAVE_INSTALL_DIR}/${SIGNED_ENCLAVE_FILENAME}")
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${SIGNED_ENCLAVE_FILENAME} DESTINATION ${ENCLAVE_INSTALL_DIR})
install(CODE "execute_process(COMMAND touch ${DATA_FILENAME})" )
install(CODE "execute_process(COMMAND chown postgres:postgres ${DATA_FILENAME})")

add_compile_definitions(ENCLAVE_FILENAME="${ENCLAVE_FILENAME}")
add_compile_definitions(DATA_FILENAME="${DATA_FILENAME}")

add_untrusted_library(SGXInterface
                    STATIC
                    SRCS ${U_SRCS} 
                    EDL ${EDL}
                    EDL_SEARCH_PATHS ${EDL_SEARCH_PATHS})
add_dependencies(SGXInterface enclave-sign)


## for extension 
add_subdirectory(extension)
