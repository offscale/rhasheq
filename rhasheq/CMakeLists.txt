get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

set(Header_Files "rhasheq.h" "rhasheq_stdbool.h")
source_group("Header Files" FILES "${Header_Files}")

add_library("${LIBRARY_NAME}" INTERFACE "${Header_Files}")

include(CheckIncludeFile)

find_package(c89stringutils CONFIG REQUIRED)

target_link_libraries("${LIBRARY_NAME}" INTERFACE c89stringutils c89stringutils_compiler_flags)

find_package(unofficial-rhash CONFIG REQUIRED)

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
    find_package(unofficial-rhash CONFIG REQUIRED)
    target_link_libraries("${LIBRARY_NAME}" PRIVATE unofficial::rhash::rhash)
endif (NOT LibRHash_FOUND)

target_link_libraries("${LIBRARY_NAME}" INTERFACE "LibRHash::LibRHash")

include(GNUInstallDirs)
target_include_directories(
        "${LIBRARY_NAME}"
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${C89STRINGUTILS_INCLUDE_DIRS}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
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
        EXPORT "${LIBRARY_NAME}Targets"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES ${Header_Files} ${extra_headers}
        TYPE "INCLUDE")
install(EXPORT "${LIBRARY_NAME}Targets" DESTINATION "${CMAKE_INSTALL_DATADIR}/${LIBRARY_NAME}")
