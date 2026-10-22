get_filename_component(EXEC_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE " " "_" EXEC_NAME "${EXEC_NAME}")

#########################
# Dependencies download #
#########################

set(GREATEST_CRC32C "c66c379f")
set(GREATEST_SHA256 "1292593d95c35eeccc89ffa1c91d6fe53b49f81cbf2c2b7758842b7f3186fcc2")
set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/test_downloads")
file(MAKE_DIRECTORY "${DOWNLOAD_DIR}")

set(GREATEST_BASEFILENAME "greatest.h")
set(GREATEST_RAW_FILE "${DOWNLOAD_DIR}/${GREATEST_BASEFILENAME}")

set(GREATEST_URL "https://raw.githubusercontent.com/silentbicycle/greatest/11a6af1/greatest.h")
if(NOT EXISTS "${GREATEST_RAW_FILE}")
    file(DOWNLOAD "${GREATEST_URL}" "${GREATEST_RAW_FILE}"
        EXPECTED_HASH "SHA256=${GREATEST_SHA256}"
    )
endif()

include(FetchContent)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../greatest")
    FetchContent_Declare(
        greatest
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../greatest"
    )
else()
    FetchContent_Declare(
        greatest
        URL "https://raw.githubusercontent.com/SamuelMarks/greatest/master/greatest.h"
        DOWNLOAD_NO_EXTRACT TRUE
    )
endif()
FetchContent_MakeAvailable(greatest)

set(GREATEST_PATCHED_DIR "${PROJECT_BINARY_DIR}/greatest_include")
file(MAKE_DIRECTORY "${GREATEST_PATCHED_DIR}")
file(COPY "${greatest_SOURCE_DIR}/greatest.h" DESTINATION "${GREATEST_PATCHED_DIR}")
if(MSVC)
    file(READ "${GREATEST_PATCHED_DIR}/${GREATEST_BASEFILENAME}" _greatest_content)
    string(REPLACE "while (0)" "while (0,0)" _greatest_content "${_greatest_content}")
    file(WRITE "${GREATEST_PATCHED_DIR}/${GREATEST_BASEFILENAME}" "${_greatest_content}")
endif()

################################
# Configure file (interpolate) #
################################

set(GREATEST_FILE "${GREATEST_RAW_FILE}")
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
    file(TO_NATIVE_PATH "${DOWNLOAD_DIR}" DOWNLOAD_DIR)
    string(REPLACE "\\" "\\\\" DOWNLOAD_DIR "${DOWNLOAD_DIR}")
    file(TO_NATIVE_PATH "${GREATEST_FILE}" GREATEST_FILE)
    string(REPLACE "\\" "\\\\" GREATEST_FILE "${GREATEST_FILE}")
endif()

configure_file(config_for_tests.h.in "${PROJECT_BINARY_DIR}/src/config_for_tests.h" @ONLY)

#########
# Tests #
#########

set(Header_Files
    "test_rhasheq.h"
    "${PROJECT_BINARY_DIR}/src/config_for_tests.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
    "test.c"
)
if(RHASHEQ_HEADER_ONLY)
    list(APPEND Source_Files "${CMAKE_SOURCE_DIR}/rhasheq/rhasheq.c")
endif()
source_group("Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

target_compile_definitions(
    "${EXEC_NAME}"
    PRIVATE
    "RHASHEQ_TEST_MOCKS"
)

target_link_libraries(
    "${EXEC_NAME}"
    PRIVATE
    "rhasheq"
    "rhasheq_compiler_flags"
)

target_include_directories(
    "${EXEC_NAME}"
    PRIVATE
    "${CMAKE_SOURCE_DIR}/rhasheq"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
    "$<BUILD_INTERFACE:${GREATEST_PATCHED_DIR}>"
)

if(EMSCRIPTEN)
    target_link_options("${EXEC_NAME}" PRIVATE "-sNODERAWFS=1")
    add_test(NAME "${EXEC_NAME}" COMMAND node "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXEC_NAME}.js")
else()
    add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")
endif()

if(RHASHEQ_BUILD_AMALGAMATION)
    add_executable("test_amalg" "${CMAKE_CURRENT_SOURCE_DIR}/test_amalg.c")
    target_compile_definitions("test_amalg" PRIVATE "RHASHEQ_EXPORTS")
    target_link_libraries(
        "test_amalg"
        PRIVATE
        "${_RHASHEQ_RHASH_TARGET}"
        c89stringutils
        "rhasheq_compiler_flags"
    )
    target_compile_definitions("test_amalg" PRIVATE "RHASHEQ_USE_C89STRINGUTILS")
    target_include_directories(
        "test_amalg"
        SYSTEM BEFORE PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/rhash_include>"
    )
    if(DEFINED c89stringutils_SOURCE_DIR)
        target_include_directories(
            "test_amalg"
            PRIVATE
            "$<BUILD_INTERFACE:${c89stringutils_SOURCE_DIR}/c89stringutils>"
            "$<BUILD_INTERFACE:${c89stringutils_BINARY_DIR}>"
            "$<BUILD_INTERFACE:${c89stringutils_BINARY_DIR}/c89stringutils>"
        )
    endif()
    target_include_directories(
        "test_amalg"
        PRIVATE
        "${CMAKE_BINARY_DIR}"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
    )
    if(EMSCRIPTEN)
        target_link_options("test_amalg" PRIVATE "-sNODERAWFS=1")
        add_test(NAME "test_amalg" COMMAND node "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_amalg.js")
    else()
        add_test(NAME "test_amalg" COMMAND "test_amalg")
    endif()
endif()
