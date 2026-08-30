add_library(payload_loader SHARED
    src/dllmain.cpp
    src/c_module.cpp
)

target_include_directories(payload_loader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/sqp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(payload_loader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/sqp/lib/secure_client_static.lib
    ws2_32
    crypt32
    user32
)

target_compile_features(payload_loader PRIVATE cxx_std_20)
