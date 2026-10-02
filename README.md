# Personal blog

## Building for Hugo with Emscripten

In the root directory:

```bash
emcmake cmake -S cpp -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-web
cmake --install build-web --prefix hugo/static
cd hugo
hugo server --buildDrafts
```

## Building SDL demos only (native)

`cd cpp` first. Then to compile:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

CMake hardcodes the path to the shaders in the source directory, so this is only for development. Emscripten embeds the shaders in the .wasm file.