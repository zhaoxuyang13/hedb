cmake_minimum_required(VERSION 3.9)
project(extension)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(SGX REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive")

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
enclave_sign(enclave OUTPUT enclave.so KEY enclave/sgx/Enclave_private.pem CONFIG enclave/sgx/enclave.config.xml)

file(GLOB U_SRCS extension/interface/sgx/*.cpp)
include_directories(extension/include extension/interface/sgx)
add_untrusted_library(SGXInterface
                    STATIC
                    SRCS ${U_SRCS} 
                    EDL ${EDL}
                    EDL_SEARCH_PATHS ${EDL_SEARCH_PATHS})
add_dependencies(SGXInterface enclave-sign)


## for extension 
add_subdirectory(extension)
