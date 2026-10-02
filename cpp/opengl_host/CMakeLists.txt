add_library(opengl_host OBJECT opengl_host.cpp)
target_link_libraries(opengl_host PUBLIC demo_core sdl3_dep)
if(NOT EMSCRIPTEN)
    target_link_libraries(opengl_host PUBLIC glad)
    target_compile_definitions(opengl_host PRIVATE
      SHADER_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../shaders")
else()
    target_link_options(opengl_host INTERFACE 
        -sMIN_WEBGL_VERSION=2 
        -sMAX_WEBGL_VERSION=2 
        --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/../shaders@/shaders
    )
    target_compile_definitions(opengl_host PRIVATE
      SHADER_SRC_DIR="/shaders")
endif()