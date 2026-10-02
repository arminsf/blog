# glad only needed for native, not emscripten.
if(NOT EMSCRIPTEN)
  add_library(glad STATIC third_party/glad/src/gl.c)
  target_include_directories(glad PUBLIC third_party/glad/include)

  find_package(OpenGL REQUIRED)
  target_link_libraries(glad PUBLIC OpenGL::GL)
endif()