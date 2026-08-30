add_library(payload_loader SHARED src/dllmain.cpp)
target_compile_features(payload_loader PRIVATE cxx_std_20)
