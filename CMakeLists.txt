cmake_minimum_required(VERSION 3.14...3.28)
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
endif()
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

project(rhasheq VERSION 0.0.2 LANGUAGES C)

string(TOUPPER "${PROJECT_NAME}" PROJECT_PREFIX)
string(REPLACE "-" "_" PROJECT_PREFIX "${PROJECT_PREFIX}")

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Control where the static and shared libraries are built so that on Windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

# Compatibility cache variables for c-ci matrix
set(CDD_CHARSET "" CACHE STRING "Charset from c-ci matrix")
set(CDD_DEPS "" CACHE STRING "Dependency provider from c-ci matrix")
set(CDD_MSVC_RTC "" CACHE STRING "MSVC runtime checks from c-ci matrix")
set(CDD_THREADING "" CACHE STRING "Threading mode from c-ci matrix")
foreach(_fc_dep AUTO-WIN-MSVC AUTO_WIN_MSVC C-ABSTRACT-HTTP C_ABSTRACT_HTTP C-ORM C_ORM C-STR-SPAN C_STR_SPAN CDD-C CDD_C CFS C_FS PARSON)
    set(FETCHCONTENT_SOURCE_DIR_${_fc_dep} "" CACHE PATH "" FORCE)
endforeach()

if(DEFINED BUILD_SHARED_LIBS)
    if(BUILD_SHARED_LIBS)
        set(RHASHEQ_BUILD_SHARED ON)
        set(RHASHEQ_BUILD_STATIC OFF)
    else()
        set(RHASHEQ_BUILD_SHARED OFF)
        set(RHASHEQ_BUILD_STATIC ON)
    endif()
else()
    option(RHASHEQ_BUILD_SHARED "Build rhasheq as a shared library" OFF)
    option(RHASHEQ_BUILD_STATIC "Build rhasheq as a static library" ON)
endif()
option(RHASHEQ_HEADER_ONLY "Use rhasheq as a header-only INTERFACE library" OFF)
option(${PROJECT_PREFIX}_BUILD_TESTING "Build tests for rhasheq" ON)
option(BUILD_TESTING "Build tests" ON)

set(${PROJECT_PREFIX}_CHARSET "Unicode" CACHE STRING "Charset to use: Unicode or ANSI")
set(${PROJECT_PREFIX}_THREADING "Multi-threaded" CACHE STRING "Threading: Multi-threaded or Single-threaded")
set(${PROJECT_PREFIX}_LTO OFF CACHE BOOL "Enable Link-Time Optimization (LTO)")
set(${PROJECT_PREFIX}_MSVC_RTC "" CACHE STRING "MSVC Runtime Checks: RTC1, RTCs, RTCu, or empty")
set(${PROJECT_PREFIX}_CRT_LINKAGE "Shared" CACHE STRING "MSVC CRT Linkage: Static or Shared")

if(NOT CDD_CHARSET STREQUAL "")
    set(${PROJECT_PREFIX}_CHARSET "${CDD_CHARSET}")
endif()
if(NOT CDD_THREADING STREQUAL "")
    set(${PROJECT_PREFIX}_THREADING "${CDD_THREADING}")
endif()
if(NOT CDD_MSVC_RTC STREQUAL "")
    set(${PROJECT_PREFIX}_MSVC_RTC "${CDD_MSVC_RTC}")
endif()

if(MSVC)
    if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY OR CMAKE_MSVC_RUNTIME_LIBRARY STREQUAL "")
        if(${PROJECT_PREFIX}_CRT_LINKAGE STREQUAL "Static" OR ${PROJECT_PREFIX}_CRT_LINKAGE STREQUAL "static")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
    endif()
endif()

if(${PROJECT_PREFIX}_LTO OR CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        if(MINGW AND NOT RHASHEQ_BUILD_SHARED)
            find_program(RHASHEQ_GCC_AR NAMES gcc-ar x86_64-w64-mingw32-gcc-ar i686-w64-mingw32-gcc-ar)
            if(RHASHEQ_GCC_AR)
                set(CMAKE_AR "${RHASHEQ_GCC_AR}" CACHE FILEPATH "Archiver" FORCE)
            endif()
            find_program(RHASHEQ_GCC_RANLIB NAMES gcc-ranlib x86_64-w64-mingw32-gcc-ranlib i686-w64-mingw32-gcc-ranlib)
            if(RHASHEQ_GCC_RANLIB)
                set(CMAKE_RANLIB "${RHASHEQ_GCC_RANLIB}" CACHE FILEPATH "Ranlib" FORCE)
            endif()
        endif()
    else()
        message(WARNING "IPO/LTO is not supported: ${ipo_error}")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
    endif()
endif()

# Strictest compiler flags
add_library("${PROJECT_NAME}_compiler_flags" INTERFACE)
if(MSVC)
    target_compile_options("${PROJECT_NAME}_compiler_flags" INTERFACE "/W4" "/WX")
    if(NOT ${PROJECT_PREFIX}_MSVC_RTC STREQUAL "" AND NOT ${PROJECT_PREFIX}_MSVC_RTC STREQUAL "OFF")
        if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
            target_compile_options("${PROJECT_NAME}_compiler_flags" INTERFACE "/${${PROJECT_PREFIX}_MSVC_RTC}")
        endif()
    endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(
        "${PROJECT_NAME}_compiler_flags"
        INTERFACE
        "-std=c89"
        "-pedantic"
        "-Werror"
        "-Wall"
        "-Wextra"
        "-Wshadow"
        "-Wformat=2"
        "-Wconversion"
        "-Wsign-conversion"
        "-Wcast-align"
        "-Wstrict-prototypes"
        "-Wold-style-definition"
        "-Wmissing-prototypes"
        "-Wmissing-declarations"
        "-Wundef"
        "-Wpointer-arith"
        "-Wcast-qual"
        "-Wwrite-strings"
    )
endif()

if(${PROJECT_PREFIX}_CHARSET STREQUAL "UNICODE" OR ${PROJECT_PREFIX}_CHARSET STREQUAL "Unicode")
    target_compile_definitions("${PROJECT_NAME}_compiler_flags" INTERFACE "RHASHEQ_UNICODE" "UNICODE" "_UNICODE")
else()
    target_compile_definitions("${PROJECT_NAME}_compiler_flags" INTERFACE "RHASHEQ_ANSI")
endif()

if(${PROJECT_PREFIX}_THREADING STREQUAL "ON" OR ${PROJECT_PREFIX}_THREADING STREQUAL "Multi-threaded")
    target_compile_definitions("${PROJECT_NAME}_compiler_flags" INTERFACE "RHASHEQ_MULTITHREADED")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
)

# Configure config.h
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.h"
)

add_subdirectory("${PROJECT_NAME}")

if((BUILD_TESTING OR ${PROJECT_PREFIX}_BUILD_TESTING) AND (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR))
    enable_testing()
    include(CTest)
    add_subdirectory("${PROJECT_NAME}_tests")
endif()

########################
# Target installation  #
########################

# Install the configuration targets
install(EXPORT "${PROJECT_NAME}Targets"
    FILE "${PROJECT_NAME}Targets.cmake"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
)

# Install the configuration file
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.h"
    TYPE "INCLUDE"
)

##############
# Packaging  #
##############

set(CPACK_PACKAGE_VENDOR "Samuel Marks")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C89 RHash file checksum verification library")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/cmake/README.txt")

set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/CPackConfig.cmake")
include(CPack)
