function(add_demo name)
  cmake_parse_arguments(DEMO "" "HOST" "SOURCES" ${ARGN})
  if(NOT DEMO_HOST)
    set(DEMO_HOST sdl_host)
  endif()

  add_library(${name}_lib STATIC ${DEMO_SOURCES})
  target_link_libraries(${name}_lib PUBLIC demo_core)

  add_executable(${name})
  target_sources(${name} PRIVATE $<TARGET_OBJECTS:${DEMO_HOST}>)
  target_link_libraries(${name} PRIVATE ${DEMO_HOST} ${name}_lib)

  if(EMSCRIPTEN)
    set_target_properties(${name} PROPERTIES SUFFIX ".js")
    target_link_options(${name} PRIVATE
      -sEXPORT_ES6=1
      -sEXPORT_NAME=create${name}
      -sENVIRONMENT=web
      -sALLOW_MEMORY_GROWTH=1)
    install(FILES "$<TARGET_FILE_DIR:${name}>/${name}.js"
                  "$<TARGET_FILE_DIR:${name}>/${name}.wasm"
            DESTINATION demos/${name})
  endif()
endfunction()