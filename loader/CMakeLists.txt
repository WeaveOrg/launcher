add_library(payload_loader SHARED
    src/dllmain.cpp
    src/c_module.cpp
    src/manualmap.cpp
)

set(HTTP2CLIENT_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/http2client")

target_include_directories(payload_loader PRIVATE
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
