get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

set(Header_Files "rhasheq.h" "rhasheq_stdbool.h" "no_discard.h")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files "rhasheq.c")
source_group("Source Files" FILES "${Source_Files}")

if(RHASHEQ_HEADER_ONLY)
    add_library("${LIBRARY_NAME}" INTERFACE)
    target_compile_definitions("${LIBRARY_NAME}" INTERFACE "RHASHEQ_HEADER_ONLY_MODE")
elseif(RHASHEQ_BUILD_SHARED)
    add_library("${LIBRARY_NAME}" SHARED "${Header_Files}" "${Source_Files}")
    target_compile_definitions("${LIBRARY_NAME}" PRIVATE "RHASHEQ_EXPORTS")
    target_compile_definitions("${LIBRARY_NAME}" PUBLIC "RHASHEQ_BUILD_SHARED")
    target_link_libraries("${LIBRARY_NAME}" PRIVATE "${PROJECT_NAME}_compiler_flags")
else()
    add_library("${LIBRARY_NAME}" STATIC "${Header_Files}" "${Source_Files}")
    target_link_libraries("${LIBRARY_NAME}" PRIVATE "${PROJECT_NAME}_compiler_flags")
endif()

include(FetchContent)

# 1. Resolve c89stringutils
set(_RHASHEQ_FOUND_C89STRINGUTILS OFF)
if(NOT CDD_DEPS STREQUAL "FETCHCONTENT")
    find_package(c89stringutils CONFIG QUIET)
    if(c89stringutils_FOUND)
        set(_RHASHEQ_FOUND_C89STRINGUTILS ON)
    endif()
endif()

if(NOT _RHASHEQ_FOUND_C89STRINGUTILS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../c89stringutils/CMakeLists.txt")
        FetchContent_Declare(
            c89stringutils
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../c89stringutils"
        )
    else()
        FetchContent_Declare(
            c89stringutils
            GIT_REPOSITORY "https://github.com/offscale/c89stringutils.git"
            GIT_TAG "master"
        )
    endif()
    set(_SAVED_BUILD_TESTING "${BUILD_TESTING}")
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(C89STRINGUTILS_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    if(APPLE)
        set(C89STRINGUTILS_STRERROR_R_CHAR_P OFF CACHE BOOL "" FORCE)
    endif()
    FetchContent_MakeAvailable(c89stringutils)
    set(BUILD_TESTING "${_SAVED_BUILD_TESTING}" CACHE BOOL "" FORCE)
    if(TARGET c89stringutils)
        if(MSVC)
            target_compile_options(c89stringutils PRIVATE "/W0")
        elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            target_compile_options(c89stringutils PRIVATE "-w")
        endif()
    endif()
    set(_RHASHEQ_FOUND_C89STRINGUTILS ON)
endif()

# 2. Resolve LibRHash
set(_RHASHEQ_RHASH_TARGET "")
set(_RHASH_ORIG_H "")
set(_RHASH_ORIG_TORRENT_H "")

if(NOT CDD_DEPS STREQUAL "FETCHCONTENT")
    find_package(unofficial-rhash CONFIG QUIET GLOBAL)
    if(unofficial-rhash_FOUND AND TARGET unofficial::rhash::rhash)
        set(_RHASHEQ_RHASH_TARGET "unofficial::rhash::rhash")
        get_target_property(_rhash_inc unofficial::rhash::rhash INTERFACE_INCLUDE_DIRECTORIES)
        if(_rhash_inc AND EXISTS "${_rhash_inc}/rhash.h")
            set(_RHASH_ORIG_H "${_rhash_inc}/rhash.h")
            if(EXISTS "${_rhash_inc}/rhash_torrent.h")
                set(_RHASH_ORIG_TORRENT_H "${_rhash_inc}/rhash_torrent.h")
            endif()
        endif()
    else()
        find_package(LibRHash QUIET GLOBAL)
        if(LibRHash_FOUND AND TARGET LibRHash::LibRHash)
            set(_RHASHEQ_RHASH_TARGET "LibRHash::LibRHash")
            if(DEFINED LibRHash_INCLUDE_DIRS AND EXISTS "${LibRHash_INCLUDE_DIRS}/rhash.h")
                set(_RHASH_ORIG_H "${LibRHash_INCLUDE_DIRS}/rhash.h")
                if(EXISTS "${LibRHash_INCLUDE_DIRS}/rhash_torrent.h")
                    set(_RHASH_ORIG_TORRENT_H "${LibRHash_INCLUDE_DIRS}/rhash_torrent.h")
                endif()
            endif()
        endif()
    endif()
endif()

if(_RHASHEQ_RHASH_TARGET STREQUAL "")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../RHash/librhash/rhash.c")
        set(_RHASH_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../RHash")
    else()
        FetchContent_Declare(
            rhash_src
            GIT_REPOSITORY "https://github.com/rhash/RHash.git"
            GIT_TAG "v1.4.6"
        )
        FetchContent_MakeAvailable(rhash_src)
        set(_RHASH_SRC_DIR "${rhash_src_SOURCE_DIR}")
    endif()

    set(_RHASH_ORIG_H "${_RHASH_SRC_DIR}/librhash/rhash.h")
    set(_RHASH_ORIG_TORRENT_H "${_RHASH_SRC_DIR}/librhash/rhash_torrent.h")

    if(NOT TARGET rhash_bundled)
        file(GLOB _RHASH_SOURCES "${_RHASH_SRC_DIR}/librhash/*.c")
        list(FILTER _RHASH_SOURCES EXCLUDE REGEX "test_lib\\.c$|test_utils\\.c$")
        if(MSVC)
            set(_RHASH_UNITY_C "${CMAKE_BINARY_DIR}/rhash_unity.c")
            set(_rhash_unity_content "/* Unity build for bundled librhash */\n")
            foreach(_rhash_src_file IN LISTS _RHASH_SOURCES)
                string(APPEND _rhash_unity_content "#include \"${_rhash_src_file}\"\n")
            endforeach()
            file(WRITE "${_RHASH_UNITY_C}" "${_rhash_unity_content}")
            add_library(rhash_bundled STATIC "${_RHASH_UNITY_C}")
        else()
            add_library(rhash_bundled STATIC ${_RHASH_SOURCES})
        endif()
        target_include_directories(
            rhash_bundled
            PRIVATE
            "${_RHASH_SRC_DIR}/librhash"
        )
        target_compile_definitions(rhash_bundled PRIVATE "RHASH_XVERSION=0x01040600" "NO_IMPORT_EXPORT")
        if(MSVC)
            target_compile_options(rhash_bundled PRIVATE "/W0")
        elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            target_compile_options(rhash_bundled PRIVATE "-std=gnu99" "-w")
        endif()
        install(TARGETS rhash_bundled
            EXPORT "${LIBRARY_NAME}Targets"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        )
    endif()
    set(_RHASHEQ_RHASH_TARGET "rhash_bundled")
endif()
set(_RHASHEQ_RHASH_TARGET "${_RHASHEQ_RHASH_TARGET}" CACHE INTERNAL "Resolved RHash target")

set(_RHASH_GEN_INCLUDE_DIR "${CMAKE_BINARY_DIR}/rhash_include")
file(MAKE_DIRECTORY "${_RHASH_GEN_INCLUDE_DIR}")
if(NOT _RHASH_ORIG_H STREQUAL "" AND EXISTS "${_RHASH_ORIG_H}")
    file(READ "${_RHASH_ORIG_H}" _rhash_h_content)
    string(REGEX REPLACE "\\}[ \t]*;(\r?\n)\\};" "} u_hash;\\1};" _rhash_h_content "${_rhash_h_content}")
    file(WRITE "${_RHASH_GEN_INCLUDE_DIR}/rhash.h" "${_rhash_h_content}")
    if(NOT _RHASH_ORIG_TORRENT_H STREQUAL "" AND EXISTS "${_RHASH_ORIG_TORRENT_H}")
        file(COPY "${_RHASH_ORIG_TORRENT_H}" DESTINATION "${_RHASH_GEN_INCLUDE_DIR}")
    endif()
    if(_RHASHEQ_RHASH_TARGET STREQUAL "rhash_bundled")
        install(FILES
            "${_RHASH_GEN_INCLUDE_DIR}/rhash.h"
            "${_RHASH_GEN_INCLUDE_DIR}/rhash_torrent.h"
            TYPE "INCLUDE"
        )
    endif()
endif()

if(RHASHEQ_HEADER_ONLY)
    set(_LINK_SCOPE INTERFACE)
else()
    set(_LINK_SCOPE PUBLIC)
endif()

target_link_libraries("${LIBRARY_NAME}" ${_LINK_SCOPE} "${_RHASHEQ_RHASH_TARGET}" c89stringutils)
target_compile_definitions("${LIBRARY_NAME}" ${_LINK_SCOPE} "RHASHEQ_USE_C89STRINGUTILS")
if(BUILD_TESTING OR ${PROJECT_PREFIX}_BUILD_TESTING)
    target_compile_definitions("${LIBRARY_NAME}" ${_LINK_SCOPE} "RHASHEQ_TEST_MOCKS")
endif()

target_include_directories(
    "${LIBRARY_NAME}"
    SYSTEM BEFORE ${_LINK_SCOPE}
    "$<BUILD_INTERFACE:${_RHASH_GEN_INCLUDE_DIR}>"
)
if(DEFINED LibRHash_INCLUDE_DIRS)
    target_include_directories(
        "${LIBRARY_NAME}"
        SYSTEM ${_LINK_SCOPE}
        "$<BUILD_INTERFACE:${LibRHash_INCLUDE_DIRS}>"
    )
endif()

if(DEFINED c89stringutils_SOURCE_DIR)
    target_include_directories(
        "${LIBRARY_NAME}"
        ${_LINK_SCOPE}
        "$<BUILD_INTERFACE:${c89stringutils_SOURCE_DIR}/c89stringutils>"
        "$<BUILD_INTERFACE:${c89stringutils_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${c89stringutils_BINARY_DIR}/c89stringutils>"
    )
endif()

include(GNUInstallDirs)
target_include_directories(
    "${LIBRARY_NAME}"
    ${_LINK_SCOPE}
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(NOT RHASHEQ_HEADER_ONLY)
    set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE C
        VERSION "${${PROJECT_NAME}_VERSION}"
        SOVERSION "${${PROJECT_NAME}_VERSION_MAJOR}"
        PUBLIC_HEADER "${Header_Files}"
    )
    include(GenerateExportHeader)
    set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
    generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")
    list(APPEND extra_headers "${_export_file}")
endif()

# Install rules
install(
    TARGETS "${LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags"
    EXPORT "${LIBRARY_NAME}Targets"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(
    FILES ${Header_Files} ${extra_headers}
    TYPE "INCLUDE"
)
install(
    EXPORT "${LIBRARY_NAME}Targets"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/${LIBRARY_NAME}"
)

option(RHASHEQ_BUILD_AMALGAMATION "Generate STB-style single-file header amalgamation" ON)
if(RHASHEQ_BUILD_AMALGAMATION)
    set(amalg_file "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_amalgamation.h")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/rhasheq_stdbool.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/no_discard.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/rhasheq.h"
    )

    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/rhasheq_stdbool.h" _amalg_stdbool)
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/no_discard.h" _amalg_nodiscard)
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/rhasheq.h" _amalg_rhasheq)
    file(READ "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.h" _amalg_config)

    string(REPLACE "#ifdef __cplusplus\nextern \"C\" {\n#endif /* __cplusplus */" "" _amalg_stdbool "${_amalg_stdbool}")
    string(REPLACE "#ifdef __cplusplus\n}\n#endif /* __cplusplus */" "" _amalg_stdbool "${_amalg_stdbool}")
    string(REPLACE "/* clang-format off */" "" _amalg_stdbool "${_amalg_stdbool}")
    string(REPLACE "/* clang-format on */" "" _amalg_stdbool "${_amalg_stdbool}")

    string(REPLACE "#ifdef __cplusplus\nextern \"C\" {\n#endif /* __cplusplus */" "" _amalg_nodiscard "${_amalg_nodiscard}")
    string(REPLACE "#ifdef __cplusplus\n}\n#endif /* __cplusplus */" "" _amalg_nodiscard "${_amalg_nodiscard}")
    string(REPLACE "/* clang-format off */" "" _amalg_nodiscard "${_amalg_nodiscard}")
    string(REPLACE "/* clang-format on */" "" _amalg_nodiscard "${_amalg_nodiscard}")

    string(REPLACE "#ifdef __cplusplus\nextern \"C\" {\n#endif /* __cplusplus */" "" _amalg_rhasheq "${_amalg_rhasheq}")
    string(REPLACE "#ifdef __cplusplus\n}\n#endif /* __cplusplus */" "" _amalg_rhasheq "${_amalg_rhasheq}")
    string(REGEX REPLACE "#include \"rhasheq_stdbool\\.h\"\r?\n" "" _amalg_rhasheq "${_amalg_rhasheq}")
    string(REGEX REPLACE "#include \"no_discard\\.h\"\r?\n" "" _amalg_rhasheq "${_amalg_rhasheq}")
    string(REGEX REPLACE "#if defined\\(__has_include\\)\r?\n#if __has_include\\(\"[^\"]+\"\\)\r?\n#endif\r?\n#elif !defined\\(RHASHEQ_SINGLE_HEADER\\)\r?\n#endif\r?\n" "" _amalg_rhasheq "${_amalg_rhasheq}")

    set(_amalgamation "/**\n * @file rhasheq_amalgamation.h\n * @brief Single-header STB-style amalgamation of rhasheq.\n */\n\n")
    string(APPEND _amalgamation "#ifndef RHASHEQ_AMALGAMATION_H\n#define RHASHEQ_AMALGAMATION_H\n\n")
    string(APPEND _amalgamation "#define RHASHEQ_SINGLE_HEADER 1\n\n")
    string(APPEND _amalgamation "${_amalg_config}\n\n")
    string(APPEND _amalgamation "#ifdef __cplusplus\nextern \"C\" {\n#endif /* __cplusplus */\n\n")
    string(APPEND _amalgamation "${_amalg_stdbool}\n\n")
    string(APPEND _amalgamation "${_amalg_nodiscard}\n\n")
    string(APPEND _amalgamation "${_amalg_rhasheq}\n\n")
    string(APPEND _amalgamation "#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n\n")
    string(APPEND _amalgamation "#endif /* !RHASHEQ_AMALGAMATION_H */\n")

    file(WRITE "${amalg_file}" "${_amalgamation}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_amalgamation.h" "${_amalgamation}")

    add_custom_target("${LIBRARY_NAME}_amalgamation" ALL
        DEPENDS "${amalg_file}"
    )

    install(
        FILES "${amalg_file}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
endif()
