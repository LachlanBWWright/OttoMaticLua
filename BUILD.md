# How to build Otto Matic

## The easy way: build.py (automated build script)

`build.py` can produce a game executable from a fresh clone of the repo in a single command. It will work on macOS, Windows and Linux, provided that your system has Python 3, CMake, and an adequate C++ compiler.

```
git clone --recurse-submodules https://github.com/jorio/OttoMatic
cd OttoMatic
python3 build.py
```

To build the **WebAssembly** (browser) version, install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) and run:

```
python3 build.py --wasm
```

The WASM bundle will be produced in `dist/OttoMatic-<version>-wasm.zip`. Extract and serve from a web server.

If you want to build the game **manually** instead, the rest of this document describes how to do just that on each of the big 3 desktop operating systems.

## How to build the game manually on macOS

1. Install the prerequisites:
    - Xcode (preferably the latest version)
    - [CMake](https://formulae.brew.sh/formula/cmake) 3.21+ (installing via Homebrew is recommended)
1. Clone the repo **recursively**:
    ```
    git clone --recurse-submodules https://github.com/jorio/OttoMatic
    cd OttoMatic
    ```
1. Download [SDL3-3.2.4.dmg](https://libsdl.org/release/SDL3-3.2.4.dmg), open it, then browse to SDL3.xcframework/macos-arm64_x86_64. In that folder, copy **SDL3.framework** to the game's **extern** folder.
1. Prep the Xcode project:
    ```
    cmake -G Xcode -S . -B build
    ```
1. Now you can open `build/OttoMatic.xcodeproj` in Xcode, or you can just go ahead and build the game:
    ```
    cmake --build build --config RelWithDebInfo
    ```
1. The game gets built in `build/RelWithDebInfo/OttoMatic.app`. Enjoy!

## How to build the game manually on Windows

1. Install the prerequisites:
    - Visual Studio 2022 with the C++ toolchain
    - [CMake](https://cmake.org/download/) 3.21+
1. Clone the repo **recursively**:
    ```
    git clone --recurse-submodules https://github.com/jorio/OttoMatic
    cd OttoMatic
    ```
1. Download [SDL3-devel-3.2.4-VC.zip](https://libsdl.org/release/SDL3-devel-3.2.4-VC.zip), extract it, and copy **SDL3-3.2.4** to the **extern** folder. Rename **SDL3-3.2.4** to just **SDL3**.
1. Prep the Visual Studio solution:
    ```
    cmake -G "Visual Studio 17 2022" -A x64 -S . -B build
    ```
1. Now you can open `build/OttoMatic.sln` in Visual Studio, or you can just go ahead and build the game:
    ```
    cmake --build build --config Release
    ```
1. The game gets built in `build/Release/OttoMatic.exe`. Enjoy!

## How to build the game manually on Linux et al.

1. Install the prerequisites from your package manager:
    - Any C++20 compiler
    - CMake 3.21+
    - SDL3 development library (e.g. "libsdl3-dev" on Ubuntu, "sdl3" on Arch, "SDL3-devel" on Fedora)
    - OpenGL development libraries (e.g. "libgl1-mesa-dev" on Ubuntu)
1. Clone the repo **recursively**:
    ```
    git clone --recurse-submodules https://github.com/jorio/OttoMatic
    cd OttoMatic
    ```
1. Build the game:
    ```
    cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build build
    ```
    If you'd like to enable runtime sanitizers, append `-DSANITIZE=1` to the **first** `cmake` call above.
1. The game gets built in `build/OttoMatic`. Enjoy!

## How to build the WebAssembly version manually

1. Install the prerequisites:
    - [Emscripten SDK (emsdk)](https://emscripten.org/docs/getting_started/downloads.html) and activate it
    - CMake 3.21+
1. Clone the repo **recursively**:
    ```
    git clone --recurse-submodules https://github.com/jorio/OttoMatic
    cd OttoMatic
    ```
1. Download SDL3 source and unpack it into `extern/SDL3-3.2.4`:
    ```
    curl -LO https://libsdl.org/release/SDL3-3.2.4.tar.gz
    tar -xzf SDL3-3.2.4.tar.gz -C extern/
    ```
1. Configure with Emscripten:
    ```
    emcmake cmake -S . -B build-wasm \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SDL_FROM_SOURCE=ON \
        -DSDL_STATIC=ON \
        -DSDL3_DIR=extern/SDL3-3.2.4
    ```
1. Build:
    ```
    cmake --build build-wasm -j$(nproc)
    ```
1. The output is `build-wasm/OttoMatic.html` (plus `.js`, `.wasm`, `.data`). Serve these files from a web server to play.

## Level editor integration

The game supports direct level loading and terrain file override for integration with level editors:

### Command-line arguments (desktop and WASM via `Module.arguments`)

- `--level N` — Skip the main menu and load level N directly (0 = Farm, 1 = Blob, etc.)
- `--terrain PATH` — Override the terrain file for the current level with a custom `.ter` file

### JavaScript API (WebAssembly only)

After the WASM module is initialized, you can call these exported functions from JavaScript:

```js
// Disable fence collision detection (useful for level editing/walkthrough)
Module.ccall('OttoMatic_SetFenceCollisions', null, ['number'], [0]);

// Re-enable fence collision detection
Module.ccall('OttoMatic_SetFenceCollisions', null, ['number'], [1]);

// Override the terrain file for the current level
// (write the .ter file to the virtual filesystem first)
FS.writeFile('/Data/Terrain/custom.ter', yourTerrainData);
Module.ccall('OttoMatic_SetTerrainPath', null, ['string'], ['/Data/Terrain/custom.ter']);
```

