add_library(sdl3_dep INTERFACE)

if(EMSCRIPTEN)
  # emcc fetches, builds and caches SDL3 itself on first use. The flag is
  # needed at compile time (headers) and at link time (the library).
  target_compile_options(sdl3_dep INTERFACE --use-port=sdl3)
  target_link_options(sdl3_dep INTERFACE --use-port=sdl3)
else()
  # Prefer a system-installed SDL3
  find_package(SDL3 3.2 QUIET CONFIG)
  if(NOT SDL3_FOUND)
    message(STATUS "System SDL3 not found; fetching it from GitHub")
    include(FetchContent)
    FetchContent_Declare(SDL3
      GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
      GIT_TAG        release-3.4.2
      GIT_SHALLOW    TRUE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON  CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL3)
  endif()
  target_link_libraries(sdl3_dep INTERFACE SDL3::SDL3)
endif()