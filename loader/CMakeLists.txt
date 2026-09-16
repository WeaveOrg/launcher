add_library(payload_loader SHARED
    src/dllmain.cpp
    src/c_module.cpp
    src/manualmap.cpp
)

set(HTTP2CLIENT_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/http2client")

target_include_directories(payload_loader PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    $<$<CONFIG:Debug>:${HTTP2CLIENT_ROOT}/debug/include>
    $<$<NOT:$<CONFIG:Debug>>:${HTTP2CLIENT_ROOT}/release/include>
)

target_link_libraries(payload_loader PRIVATE
    $<$<CONFIG:Debug>:${HTTP2CLIENT_ROOT}/debug/lib/http2client.lib>
    $<$<CONFIG:Debug>:${HTTP2CLIENT_ROOT}/debug/lib/nghttp2.lib>
    $<$<CONFIG:Debug>:${HTTP2CLIENT_ROOT}/debug/lib/wolfssl.lib>
    $<$<CONFIG:Debug>:${HTTP2CLIENT_ROOT}/debug/lib/zlib.lib>
    $<$<NOT:$<CONFIG:Debug>>:${HTTP2CLIENT_ROOT}/release/lib/http2client.lib>
    $<$<NOT:$<CONFIG:Debug>>:${HTTP2CLIENT_ROOT}/release/lib/nghttp2.lib>
    $<$<NOT:$<CONFIG:Debug>>:${HTTP2CLIENT_ROOT}/release/lib/wolfssl.lib>
    $<$<NOT:$<CONFIG:Debug>>:${HTTP2CLIENT_ROOT}/release/lib/zlib.lib>
    ws2_32
    crypt32
    user32
)

target_compile_features(payload_loader PRIVATE cxx_std_23)

if(MSVC)
    target_link_options(payload_loader PRIVATE /MAP)
endif()

set(VMPROTECT_CON "${CMAKE_SOURCE_DIR}/tools/VMProtect_Con.exe" CACHE FILEPATH
    "Path to the VMProtect console executable")

if(NOT EXISTS "${VMPROTECT_CON}")
    message(FATAL_ERROR
        "VMProtect console executable was not found at '${VMPROTECT_CON}'. "
        "Set VMPROTECT_CON to the correct path when configuring CMake.")
endif()

set(VMPROTECT_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/loader.dll")

add_custom_command(
    OUTPUT "${VMPROTECT_OUTPUT}"
    COMMAND "${VMPROTECT_CON}"
            "$<TARGET_FILE:payload_loader>"
            "${VMPROTECT_OUTPUT}"
    DEPENDS payload_loader
    COMMENT "Protecting payload_loader.dll with VMProtect"
    VERBATIM
)

add_custom_target(protect_loader ALL
    DEPENDS "${VMPROTECT_OUTPUT}"
)
