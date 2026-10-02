cmake_minimum_required(VERSION 3.25)
project(blog_demos LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Third-party first, so our warning flags below don't apply to SDL's own sources
include(cmake/SDL3.cmake)
include(cmake/glad.cmake)
include(cmake/AddDemo.cmake)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra)
endif()

add_subdirectory(core)
add_subdirectory(sdl_host)
add_subdirectory(opengl_host)
add_subdirectory(demos/mercator)