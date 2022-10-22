cmake_minimum_required(VERSION 3.23)
project(git_contributions_stats)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    add_compile_options(-Wall -Wextra)
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    if(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "15.0.0" )
        add_compile_definitions(HAS_CLANG_SOURCE_LOCATION)
    endif()
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    if(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "11.1.0" )
        add_compile_definitions(HAS_GCC)
    endif()
endif()

find_path(GIT2_INCLUDE_PATH NAMES git2.h
        HINTS /usr/include
        )
# GIT2_LIBRARY
find_library(GIT2_LIBRARY NAMES git2
        HINTS /lib64
        )
include_directories(${GIT2_INCLUDE_PATH})

add_executable(git_contributions_stats main.cpp Repository.h Commit.h utils.h)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT output)
if(ipo_supported)
    set_property(TARGET git_contributions_stats PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO is not supported: ${output}")
endif()

target_link_libraries(git_contributions_stats ${GIT2_LIBRARY})
