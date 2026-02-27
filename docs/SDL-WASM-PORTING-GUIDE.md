# Porting SDL Games to WebAssembly (Emscripten)

A comprehensive guide for porting SDL2/SDL3-based C/C++ games to run in the browser via Emscripten and WebAssembly (WASM).

## Table of Contents

1. [Overview](#overview)
2. [Build System Setup](#build-system-setup)
3. [Main Loop Architecture](#main-loop-architecture)
4. [OpenGL / WebGL Compatibility](#opengl--webgl-compatibility)
5. [Exception Handling](#exception-handling)
6. [Asset Loading & Virtual Filesystem](#asset-loading--virtual-filesystem)
7. [Audio](#audio)
8. [Input Handling](#input-handling)
9. [Memory Management](#memory-management)
10. [HTML Shell & JavaScript Integration](#html-shell--javascript-integration)
11. [Debugging & Troubleshooting](#debugging--troubleshooting)
12. [CI/CD Deployment](#cicd-deployment)
13. [Common Pitfalls](#common-pitfalls)

---

## Overview

Emscripten compiles C/C++ code to WebAssembly, which runs in the browser. SDL provides a cross-platform abstraction layer that Emscripten supports natively. However, the browser environment has fundamental differences from native platforms:

- **No blocking**: The browser's main thread must not be blocked for long periods.
- **WebGL only**: Browsers support WebGL (based on OpenGL ES 2.0/3.0), not desktop OpenGL.
- **Virtual filesystem**: There is no real filesystem; assets must be preloaded or fetched.
- **Single-threaded**: The main thread is shared with the browser UI.

## Build System Setup

### CMake Configuration

Use `emcmake` to wrap CMake so it picks up the Emscripten toolchain:

```bash
emcmake cmake -S . -B build-wasm \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SDL_FROM_SOURCE=ON \
  -DSDL_STATIC=ON
```

SDL3 must be built from source for Emscripten (static library only):

```cmake
if(EMSCRIPTEN)
    # SDL3 source must be available
    set(SDL_STATIC ON)
    add_subdirectory(extern/SDL3-source)
endif()
```

### Key Emscripten Link Flags

```cmake
if(EMSCRIPTEN)
    target_link_options(${TARGET} PRIVATE
        # ASYNCIFY: Allows blocking C loops to yield to the browser event loop.
        # Required if your game uses while-loops instead of emscripten_set_main_loop.
        "SHELL:-sASYNCIFY=1"
        # Increase stack size for complex games (default 4096 is too small).
        "SHELL:-sASYNCIFY_STACK_SIZE=65536"
        # Enable C++ exception catching (JS-based, compatible with ASYNCIFY).
        "SHELL:-sDISABLE_EXCEPTION_CATCHING=0"
        # Emulate legacy OpenGL fixed-function pipeline via WebGL shaders.
        "SHELL:-sLEGACY_GL_EMULATION=1"
        # Allow heap to grow dynamically.
        "SHELL:-sALLOW_MEMORY_GROWTH=1"
        # Initial heap size (256 MB example for a large game).
        "SHELL:-sINITIAL_MEMORY=268435456"
        # Preload game assets into the virtual filesystem.
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/Data@/Data"
        # Export functions callable from JavaScript.
        "SHELL:-sEXPORTED_FUNCTIONS=['_main']"
        "SHELL:-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS']"
        # Custom HTML shell template.
        "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/shell.html"
    )
endif()
```

## Main Loop Architecture

### The Problem

Browsers require the main thread to return control frequently. A traditional game loop:

```c
while (running) {
    processInput();
    update();
    render();
}
```

...will **freeze the browser tab**.

### Solution A: `emscripten_set_main_loop` (Recommended for new code)

Restructure the loop into a callback:

```c
void mainloop(void) {
    processInput();
    update();
    render();
}

int main() {
    init();
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 1);
#else
    while (running) mainloop();
#endif
}
```

### Solution B: ASYNCIFY with `emscripten_sleep` (For porting existing code)

If restructuring every loop is impractical, use ASYNCIFY. This allows existing blocking loops to work by yielding to the browser:

```c
// In game.h - portable yield macro
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GAME_YIELD_BROWSER() emscripten_sleep(0)
#else
#define GAME_YIELD_BROWSER() ((void)0)
#endif
```

Add `GAME_YIELD_BROWSER()` to **every** blocking while/do-while loop:

```c
while (gameRunning) {
    GAME_YIELD_BROWSER();  // Must be first in the loop
    processInput();
    update();
    render();
}
```

**Critical**: You must add yields to ALL blocking loops, including:
- Main game loop
- Menu loops
- Screen transition loops (fade in/out)
- Score entry loops
- Cutscene/intro loops
- Any busy-wait loops

Missing even one loop will freeze the browser.

### Solution C: SDL3 Main Callbacks (Best for SDL3)

SDL3 provides built-in callback infrastructure:

```c
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]);
SDL_AppResult SDL_AppIterate(void *appstate);
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event);
void SDL_AppQuit(void *appstate, SDL_AppResult result);
```

SDL handles `emscripten_set_main_loop` internally.

## OpenGL / WebGL Compatibility

### WebGL Limitations

WebGL is based on OpenGL ES 2.0, which lacks:

- **Fixed-function pipeline**: No `glBegin`/`glEnd`, `glVertex`, `glColor`, etc.
- **`GL_QUADS`**: Not supported; convert to triangles.
- **`glClientActiveTexture`**: Not available.
- **Matrix stack**: No `glPushMatrix`/`glPopMatrix`.
- **Alpha test**: No `glAlphaFunc`; implement in shaders.
- **Fixed-function lighting**: Must be done in shaders.

### Approach 1: Use `LEGACY_GL_EMULATION`

Add `-sLEGACY_GL_EMULATION=1` to link flags. Emscripten provides limited emulation of legacy GL calls. This is the quickest path but has limitations and performance overhead.

### Approach 2: Custom Compatibility Layer (More Robust)

Create a shader-based compatibility layer that intercepts legacy GL calls:

```c
// gl_compat.h - Redirect legacy calls to modern implementations
#ifdef __EMSCRIPTEN__
#define glBegin(mode) ModernGL_BeginImmediateMode(mode)
#define glEnd()       ModernGL_EndImmediateMode()
#define glVertex3f(x,y,z) ModernGL_ImmediateVertex(x,y,z)
#define glColor4f(r,g,b,a) ModernGL_ImmediateColor(r,g,b,a)
// ... etc
#endif
```

Implement these with a vertex buffer + shader program.

### OpenGL ES Context Setup

```c
#ifdef __EMSCRIPTEN__
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
```

## Exception Handling

### Critical: ASYNCIFY + Exception Compatibility

**`-fwasm-exceptions` is NOT compatible with ASYNCIFY.** If you use ASYNCIFY, you must use JS-based exception handling instead:

```cmake
# WRONG: Will cause silent failures with ASYNCIFY
if(EMSCRIPTEN)
    add_compile_options(-fwasm-exceptions)
    add_link_options(-fwasm-exceptions)
endif()

# CORRECT: JS-based exceptions, compatible with ASYNCIFY
if(EMSCRIPTEN)
    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)
    target_link_options(${TARGET} PRIVATE "SHELL:-sDISABLE_EXCEPTION_CATCHING=0")
endif()
```

The `-fexceptions` flag must be set globally (before `add_subdirectory` calls) so all translation units use the same ABI.

### If You Don't Need ASYNCIFY

If using `emscripten_set_main_loop` instead of ASYNCIFY, you can safely use `-fwasm-exceptions` for better performance.

## Asset Loading & Virtual Filesystem

### Preloading Assets

Use `--preload-file` to embed assets into a `.data` file:

```cmake
"SHELL:--preload-file ${CMAKE_SOURCE_DIR}/Data@/Data"
```

This creates `Game.data` alongside the `.wasm` file. The virtual filesystem mounts files at the specified path (`/Data`).

### Large Asset Files

For games with large assets (>50MB), consider:

- **Lazy loading**: Use `--use-preload-plugins` and fetch assets on demand.
- **Compression**: The `.data` file can be gzip-compressed by the web server.
- **Splitting**: Break assets into multiple packages loaded progressively.

### Config Directory

Create the config directory in `preRun`:

```javascript
Module.preRun = [function() {
    FS.mkdir('/home');
    FS.mkdir('/home/web_user');
    FS.mkdir('/home/web_user/.config');
    FS.mkdir('/home/web_user/.config/MyGame');
}];
```

## Audio

SDL's audio backend for Emscripten uses the Web Audio API. Key considerations:

- **Autoplay policy**: Browsers block audio until user interaction. Start audio on first click/keypress.
- **ScriptProcessorNode deprecation**: SDL currently uses the deprecated ScriptProcessorNode. This produces console warnings but works fine. SDL will migrate to AudioWorklet in a future release.
- **Sample rates**: Web Audio may resample; ensure your audio files are compatible.

## Input Handling

- **Mouse capture**: `SDL_SetRelativeMouseMode` works but requires user gesture (click) to engage.
- **Keyboard**: All standard SDL keyboard events work. Be aware of browser shortcuts (Ctrl+W, etc.).
- **Gamepad**: SDL gamepad support works via the Gamepad API.
- **Touch**: SDL touch events map to browser touch events.
- **Non-passive event listeners**: SDL registers non-passive listeners for wheel/touch events to prevent default browser behavior. This produces console warnings but is necessary for game input.

## Memory Management

```cmake
# Allow heap to grow (essential for games that allocate dynamically)
"SHELL:-sALLOW_MEMORY_GROWTH=1"
# Set initial memory (must be multiple of 64KB page size)
"SHELL:-sINITIAL_MEMORY=268435456"  # 256 MB
```

If you see `Cannot enlarge memory arrays` errors, increase `INITIAL_MEMORY` or ensure `ALLOW_MEMORY_GROWTH=1` is set.

## HTML Shell & JavaScript Integration

### Custom Shell Template

Create a shell HTML file with the Emscripten Module object:

```html
<canvas id="canvas" tabindex="-1"></canvas>
<script>
var Module = {
    canvas: document.getElementById('canvas'),
    print: function(t) { console.log('[Game]', t); },
    printErr: function(t) { console.warn('[Game stderr]', t); },
    onAbort: function(what) { console.error('[Game] ABORT:', what); },
    onRuntimeInitialized: function() { console.log('Runtime ready'); },
    setStatus: function(text) { /* Update loading UI */ },
    preRun: [function() { /* Setup virtual FS */ }]
};
</script>
<script async src="Game.js"></script>
```

### Exporting C Functions to JavaScript

```c
// In C code
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EMSCRIPTEN_KEEPALIVE void MyExportedFunction(int param) { /* ... */ }
#endif
```

```cmake
"SHELL:-sEXPORTED_FUNCTIONS=['_main','_MyExportedFunction']"
"SHELL:-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS']"
```

```javascript
// Call from JavaScript
Module.ccall('MyExportedFunction', null, ['number'], [42]);
```

## Debugging & Troubleshooting

### Verbose Logging

Always enable verbose logging for Emscripten builds:

```c
#ifdef __EMSCRIPTEN__
SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
#endif
```

### Common Errors

| Error | Cause | Fix |
|-------|-------|-----|
| `ASYNCIFY=1 is not compatible with -fwasm-exceptions` | Using both flags | Use `-fexceptions` + `-sDISABLE_EXCEPTION_CATCHING=0` instead |
| Page freezes/unresponsive | Blocking loop without yield | Add `emscripten_sleep(0)` to all blocking loops |
| `unreachable executed` | ASYNCIFY_STACK_SIZE too small | Increase to 65536 or higher |
| `function signature mismatch` | ABI mismatch in exception handling | Ensure all TUs use same exception flags |
| `Cannot enlarge memory arrays` | Heap too small | Set `ALLOW_MEMORY_GROWTH=1` and increase `INITIAL_MEMORY` |
| Black screen, no GL errors | GL context not created | Check `SDL_GL_CONTEXT_PROFILE_ES` is set |
| `WARNING: using emscripten GL emulation` | `LEGACY_GL_EMULATION=1` active | Expected warning; safe to ignore if game renders correctly |

### Browser Developer Tools

- **Console**: Check for JavaScript errors and WASM traps.
- **Network tab**: Verify `.data`, `.wasm`, and `.js` files load correctly.
- **Performance tab**: Profile to find bottlenecks in WASM execution.

## CI/CD Deployment

### GitHub Pages Workflow

```yaml
- name: Install Emscripten
  run: |
    git clone --depth=1 https://github.com/emscripten-core/emsdk.git .emsdk
    .emsdk/emsdk install latest
    .emsdk/emsdk activate latest
    source .emsdk/emsdk_env.sh

- name: Build WASM
  run: |
    emcmake cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

- name: Assemble site
  run: |
    mkdir -p _site
    cp build/Game.html _site/index.html
    cp build/Game.js build/Game.wasm build/Game.data _site/
    touch _site/.nojekyll
```

### Important Notes

- Add `.nojekyll` to prevent GitHub Pages from processing files.
- The `.data` file can be very large (100MB+). GitHub Pages has a 1GB limit per site.
- Cache the Emscripten SDK in CI to speed up builds.

## Common Pitfalls

1. **Forgetting to yield in ALL loops**: Even one missing yield freezes the browser.
2. **Mixing `-fwasm-exceptions` with ASYNCIFY**: They are incompatible. Choose one.
3. **Not setting OpenGL ES profile**: Desktop GL calls will fail silently on WebGL.
4. **Blocking during initialization**: Long init phases block the browser. Add yields between heavy init steps.
5. **Assuming filesystem access**: All file I/O goes through Emscripten's virtual FS.
6. **Not testing with different browsers**: WebGL support varies between Chrome, Firefox, and Safari.
7. **Forgetting to set exception flags globally**: All translation units (including libraries like Pomme, SDL) must use the same exception-handling ABI.

---

*This guide was created as part of the OttoMatic WebAssembly port. For the reference implementation, see the [OttoMatic-Android repository](https://github.com/LachlanBWWright/OttoMatic-Android).*
