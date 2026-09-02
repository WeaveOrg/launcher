cmake_minimum_required(VERSION 3.20)
project(WeaveLauncher VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/webview2_pkg/build/native/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/http2client/release/include
    ${CMAKE_CURRENT_SOURCE_DIR}/loader/src
)

# Source files
set(SOURCES
    src/main.cpp
    src/hwid.cpp
    src/process_manager.cpp
    src/system_info.cpp
    src/loader.cpp
)

# WIN32 creates a pure Windows GUI application without console window
add_executable(weave_launcher WIN32 ${SOURCES})

if(WIN32)
    target_link_libraries(weave_launcher PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/webview2_pkg/build/native/x64/WebView2Loader.dll.lib
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/http2client/debug/lib/http2client.lib>
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/http2client/debug/lib/nghttp2.lib>
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/http2client/debug/lib/wolfssl.lib>
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/http2client/debug/lib/zlib.lib>
        $<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/http2client/release/lib/http2client.lib>
        $<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/http2client/release/lib/nghttp2.lib>
        $<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/http2client/release/lib/wolfssl.lib>
        $<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/http2client/release/lib/zlib.lib>
        ws2_32
        crypt32
        iphlpapi
        advapi32
        user32
        gdi32
        shell32
        ole32
        oleaut32
        dwmapi
        shlwapi
        version
    )

    if(MINGW)
        target_compile_options(weave_launcher PRIVATE -mstackrealign -mwindows)
        target_link_options(weave_launcher PRIVATE -mstackrealign -mwindows)
    endif()

    # Copy WebView2Loader.dll next to the executable
    add_custom_command(TARGET weave_launcher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/webview2_pkg/build/native/x64/WebView2Loader.dll"
        $<TARGET_FILE_DIR:weave_launcher>
    )
endif()

include(FetchContent)
FetchContent_Declare(
    saucer
    GIT_REPOSITORY https://github.com/saucer/saucer.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(saucer)

FetchContent_Declare(
    saucer-loop
    GIT_REPOSITORY https://github.com/saucer/loop.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(saucer-loop)

target_link_libraries(weave_launcher PRIVATE saucer::saucer saucer::loop)

# Add internal payload DLL loader
add_subdirectory(loader)
