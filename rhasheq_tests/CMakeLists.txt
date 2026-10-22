cmake_minimum_required(VERSION 3.14...3.28)
project(rhasheq_tests
        VERSION "0.0.2"
        LANGUAGES C)

set(HOMEPAGE_URL "https://github.com/offscale/rhasheq")
set(DESCRIPTION "Tests for rhasheq")
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

add_subdirectory("test_rhasheq")
