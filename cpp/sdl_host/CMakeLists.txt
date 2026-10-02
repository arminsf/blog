add_library(sdl_host OBJECT sdl_host.cpp)
target_link_libraries(sdl_host PUBLIC demo_core sdl3_dep)