get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE " " "_" LIBRARY_NAME "${LIBRARY_NAME}")

set(Header_Files "rhasheq.h" "rhasheq_stdbool.h")

source_group("Header Files" FILES "${Header_Files}")

add_library("${LIBRARY_NAME}" INTERFACE "${Header_Files}")

include(CheckIncludeFile)

if (CMAKE_C_STANDARD GREATER 90)
    CHECK_INCLUDE_FILE("stdbool.h" HAS_STDBOOL)
    if (HAS_STDBOOL)
        add_compile_definitions(HAS_STDBOOL=1)
    endif (HAS_STDBOOL)
endif (CMAKE_C_STANDARD GREATER 90)

# TODO: Move this to vcpkg… I guess?
set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/third_party")
file(MAKE_DIRECTORY "${DOWNLOAD_DIR}")
set(extra_headers "${DOWNLOAD_DIR}/c89stringutils_string_extras.h")

file(DOWNLOAD "https://raw.githubusercontent.com/offscale/c89stringutils/992cace/c89stringutils/c89stringutils_string_extras.h"
        "${extra_headers}")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules;${CMAKE_MODULE_PATH}")
find_package(LibRHash)

if (NOT LibRHash_FOUND)
    set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} lib)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES} _imp.lib -d.lib -d_imp.lib)
    find_package(PkgConfig)
    include(FindPkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(LibRHash librhash)
    endif (PKG_CONFIG_FOUND)
endif (NOT LibRHash_FOUND)
if (NOT TARGET LibRHash)
    find_package(LibRHash REQUIRED)
endif (NOT TARGET LibRHash)
if (NOT LibRHash_FOUND)
    message(FATAL_ERROR "LibRHash not found")
endif (NOT LibRHash_FOUND)

target_link_libraries("${LIBRARY_NAME}" INTERFACE "LibRHash::LibRHash")

target_include_directories(
        "${LIBRARY_NAME}"
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
        "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)



set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

# install rules
set(installable_libs "${LIBRARY_NAME}")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "${INSTALL_DEST}"
        EXPORT "${LIBRARY_NAME}Targets")
install(FILES ${Header_Files} ${extra_headers} DESTINATION "include")
