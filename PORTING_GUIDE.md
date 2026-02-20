# Porting Desktop OpenGL Games to Android

A practical guide based on the experience of porting **Otto Matic** (a C/C++ SDL game using fixed-function OpenGL) to Android with OpenGL ES 3.0.

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Project Structure](#project-structure)
4. [Build System Setup](#build-system-setup)
5. [SDL3 Android Integration](#sdl3-android-integration)
6. [OpenGL ES Migration](#opengl-es-migration)
7. [Touch Controls](#touch-controls)
8. [Common Pitfalls](#common-pitfalls)
9. [Testing & Debugging](#testing--debugging)
10. [Checklist](#checklist)

---

## Overview

Porting a desktop OpenGL game to Android involves several major challenges:

- **OpenGL → OpenGL ES**: Desktop games often use the fixed-function pipeline (OpenGL 1.x/2.x), which doesn't exist in modern OpenGL ES. You need to either rewrite all rendering to use shaders, or create a compatibility bridge.
- **Input**: Keyboard/mouse/gamepad must be replaced or supplemented with touch controls.
- **Build System**: The game must compile with the Android NDK and be packaged as an APK.
- **Platform Differences**: File paths, screen orientation, lifecycle management, and other platform-specific details.

This guide assumes your game uses **SDL** (Simple DirectMedia Layer) and **OpenGL**, which is the most common setup for cross-platform C/C++ games.

---

## Prerequisites

- **Android Studio** (or at minimum the Android SDK, NDK, and CMake)
- **Android NDK** r25+ (this guide uses r27)
- **CMake** 3.22+
- **JDK 17** (for Gradle)
- **SDL3** source code (cloned from https://github.com/libsdl-org/SDL)
- The game's source code with a CMake-based build system

---

## Project Structure

A typical ported project has this layout:

```
GameRoot/
├── CMakeLists.txt              # Main CMake build (shared between desktop & Android)
├── android/
│   ├── app/
│   │   ├── build.gradle.kts    # Android build configuration
│   │   ├── src/main/
│   │   │   ├── AndroidManifest.xml
│   │   │   ├── java/org/libsdl/app/   # SDL Java bridge files
│   │   │   └── res/                    # Icons, themes, etc.
│   ├── build.gradle.kts        # Root Gradle config
│   ├── settings.gradle.kts
│   ├── gradle.properties
│   └── gradlew / gradlew.bat
├── extern/
│   ├── SDL/                    # SDL3 source (cloned, not committed)
│   └── OtherLibs/
├── src/                        # Game source code
│   ├── 3D/                     # Rendering code
│   ├── Headers/
│   └── System/
└── Data/                       # Game assets
```

### Key Files to Create

#### `android/app/build.gradle.kts`
```kotlin
plugins {
    id("com.android.application")
}

android {
    namespace = "com.example.yourgame"
    compileSdk = 34
    ndkVersion = "27.3.13750724"

    defaultConfig {
        applicationId = "com.example.yourgame"
        minSdk = 24        // Android 7.0 - supports GLES 3.0
        targetSdk = 34
        versionCode = 1
        versionName = "1.0.0"

        ndk {
            abiFilters += listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")
        }

        externalNativeBuild {
            cmake {
                arguments(
                    "-DANDROID_STL=c++_shared",
                    "-DBUILD_SDL_FROM_SOURCE=ON",
                    "-DSDL_STATIC=OFF",
                    "-DANDROID=TRUE"
                )
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")  // Points to your main CMakeLists
            version = "3.22.1"
        }
    }

    sourceSets {
        getByName("main") {
            assets.srcDirs("../../Data")  // Your game data directory
        }
    }
}
```

#### `android/app/src/main/AndroidManifest.xml`
```xml
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android">
    <uses-feature android:glEsVersion="0x00030000" android:required="true" />
    <uses-feature android:name="android.hardware.touchscreen" android:required="false" />

    <application
        android:label="YourGame"
        android:icon="@mipmap/ic_launcher"
        android:hasCode="true"
        android:theme="@android:style/Theme.NoTitleBar.Fullscreen">

        <activity
            android:name="org.libsdl.app.SDLActivity"
            android:exported="true"
            android:configChanges="orientation|screenSize|keyboard|keyboardHidden"
            android:screenOrientation="sensorLandscape">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
```

---

## Build System Setup

### CMakeLists.txt Modifications

Add Android detection and configuration to your main CMakeLists.txt:

```cmake
if(ANDROID)
    # Force SDL to build from source on Android
    set(BUILD_SDL_FROM_SOURCE ON CACHE BOOL "Build SDL from source for Android" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "Use shared SDL on Android" FORCE)
    add_compile_definitions(__ANDROID__)
endif()

# SDL from source (required for Android)
if(BUILD_SDL_FROM_SOURCE)
    if(NOT DEFINED SDL3_DIR)
        set(SDL3_DIR "${CMAKE_SOURCE_DIR}/extern/SDL")
    endif()
    add_subdirectory("${SDL3_DIR}" EXCLUDE_FROM_ALL)
endif()

# Your game target
add_library(${GAME_TARGET} SHARED ...)  # SHARED, not EXECUTABLE, for Android

# Link appropriate OpenGL ES library on Android
if(ANDROID)
    target_link_libraries(${GAME_TARGET} PRIVATE GLESv3 log android)
else()
    # Desktop OpenGL linking
    find_package(OpenGL REQUIRED)
    target_link_libraries(${GAME_TARGET} PRIVATE OpenGL::GL)
endif()
```

**Important**: On Android, your game must be a **shared library** (`SHARED`), not an executable. SDL's Java layer loads it via `System.loadLibrary()`.

---

## SDL3 Android Integration

### Getting SDL3 Java Files

SDL3 requires Java bridge files in your Android project. Copy them from the SDL source:

```bash
mkdir -p android/app/src/main/java/org/libsdl/app
cp extern/SDL/android-project/app/src/main/java/org/libsdl/app/*.java \
   android/app/src/main/java/org/libsdl/app/
```

These files handle:
- Activity lifecycle
- Surface creation/destruction
- Touch event routing
- Audio initialization
- Sensor access

### SDL Entry Point

Your game's `main()` function is called by SDL's Java bridge automatically. No changes needed to your main function — SDL handles the JNI bridging.

### Boot/Initialization

When creating the SDL window on Android, request a GLES 3.0 context:

```c
#ifdef __ANDROID__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
```

---

## OpenGL ES Migration

This is the most complex part. There are two approaches:

### Approach A: Rewrite All Rendering (Clean but Expensive)

Rewrite every `glBegin`/`glEnd` call, every matrix operation, every lighting setup to use modern shaders. This is the "right" way but requires touching every rendering file.

### Approach B: Compatibility Bridge (Pragmatic)

Create a **bridge layer** that emulates the fixed-function pipeline using shaders. This is what we did for Otto Matic.

#### The Bridge Architecture

```
Game Code → gles_compat.h (macros) → GLESBridge.c (emulation) → GLES 3.0
```

1. **`gles_compat.h`**: A header that `#define`s all fixed-function GL calls to bridge functions
2. **`GLESBridge.c`**: Implements the bridge using GLES 3.0 shaders and VBOs

#### What the Bridge Must Handle

| Fixed-Function Feature | Bridge Implementation |
|----------------------|----------------------|
| `glBegin`/`glEnd` (immediate mode) | Buffer vertices, flush on `glEnd` using VBOs |
| `glVertex3f`, `glNormal3f`, `glTexCoord2f`, `glColor4f` | Accumulate in CPU-side arrays |
| `glMatrixMode`, `glPushMatrix`, `glPopMatrix` | Software matrix stacks (modelview, projection, texture) |
| `glLoadIdentity`, `glTranslatef`, `glRotatef`, `glScalef` | CPU-side matrix math |
| `glOrtho`, `glFrustum` | CPU-side projection matrix construction |
| `glMultMatrixf` | CPU-side matrix multiplication |
| `glEnableClientState`, `glVertexPointer`, etc. | Track state, upload to VBOs before draw |
| `glDrawElements` with client-side arrays | Upload to streaming VBOs, then draw |
| `glLightfv`, `glMaterialfv` | Pass as uniforms to uber-shader |
| `glFogfv`, `glFogf` | Pass fog parameters as uniforms |
| `glAlphaFunc` | Implement via `discard` in fragment shader |
| `glTexEnvi` | Texture environment mode as uniform |
| `GL_QUADS`, `GL_QUAD_STRIP` | Convert to triangles |
| Multi-texture (`glActiveTexture` + `GL_TEXTURE1`) | Second sampler uniform in shader |
| `glTexGeni` (sphere mapping) | Compute in vertex shader |

#### The Uber-Shader

Create a single vertex/fragment shader pair that handles all rendering modes via uniforms:

```glsl
// Vertex Shader (simplified)
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_textureMatrix;
uniform bool u_lightingEnabled;
uniform bool u_texGenEnabled;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;
attribute vec4 a_color;

varying vec4 v_color;
varying vec2 v_texcoord;

void main() {
    gl_Position = u_mvpMatrix * vec4(a_position, 1.0);

    if (u_lightingEnabled) {
        // Compute lighting in eye space
        vec3 eyeNormal = normalize(mat3(u_modelViewMatrix) * a_normal);
        // ... accumulate light contributions
    } else {
        v_color = a_color;
    }

    if (u_texGenEnabled) {
        // Sphere map reflection
        vec3 eyePos = (u_modelViewMatrix * vec4(a_position, 1.0)).xyz;
        vec3 reflection = reflect(normalize(eyePos), eyeNormal);
        // ... compute tex coords from reflection
    } else {
        v_texcoord = (u_textureMatrix * vec4(a_texcoord, 0.0, 1.0)).xy;
    }
}
```

```glsl
// Fragment Shader (simplified)
uniform sampler2D u_texture0;
uniform bool u_textureEnabled;
uniform bool u_alphaTestEnabled;
uniform int u_alphaFunc;
uniform float u_alphaRef;
uniform bool u_fogEnabled;

varying vec4 v_color;
varying vec2 v_texcoord;

void main() {
    vec4 color = v_color;

    if (u_textureEnabled) {
        color *= texture2D(u_texture0, v_texcoord);
    }

    if (u_alphaTestEnabled) {
        // GL_GREATER, GL_GEQUAL, GL_LESS, GL_LEQUAL, GL_EQUAL, GL_NOTEQUAL
        if (u_alphaFunc == 0x0204 && color.a <= u_alphaRef) discard; // GL_GREATER
        // ... other comparisons
    }

    if (u_fogEnabled) {
        // Linear fog
        float fogFactor = clamp((u_fogEnd - gl_FragCoord.z/gl_FragCoord.w) / (u_fogEnd - u_fogStart), 0.0, 1.0);
        color.rgb = mix(u_fogColor.rgb, color.rgb, fogFactor);
    }

    gl_FragColor = color;
}
```

#### Critical: VBO Usage on Android

**Never use client-side vertex arrays on Android GLES 3.0.** Many drivers produce `GL_INVALID_OPERATION` (error 0x502) when you call `glDrawElements` with a non-zero VAO and client-side pointers.

Always upload data to streaming VBOs:

```c
// BAD (works on desktop, crashes on Android):
glVertexPointer(3, GL_FLOAT, stride, clientSideArray);
glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, clientSideIndices);

// GOOD:
glBindBuffer(GL_ARRAY_BUFFER, streamingVBO);
glBufferData(GL_ARRAY_BUFFER, dataSize, clientSideArray, GL_STREAM_DRAW);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, streamingIBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, clientSideIndices, GL_STREAM_DRAW);
glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0);
```

#### Texture Format Conversion

GLES 3.0 doesn't support `GL_LUMINANCE` or `GL_LUMINANCE_ALPHA` as internal formats. Convert them:

```c
#ifdef __ANDROID__
if (internalFormat == GL_LUMINANCE)
    internalFormat = GL_R8;  // or expand to GL_RGBA
if (internalFormat == GL_LUMINANCE_ALPHA)
    internalFormat = GL_RG8; // or expand to GL_RGBA
#endif
```

#### The `gles_compat.h` Redirect Header

```c
#ifdef __ANDROID__
#include "gles_bridge.h"

// Redirect all fixed-function calls to bridge
#define glBegin             bridge_Begin
#define glEnd               bridge_End
#define glVertex3f          bridge_Vertex3f
#define glNormal3f          bridge_Normal3f
#define glTexCoord2f        bridge_TexCoord2f
#define glColor4f           bridge_Color4f
#define glColor4fv          bridge_Color4fv
#define glMatrixMode        bridge_MatrixMode
#define glPushMatrix        bridge_PushMatrix
#define glPopMatrix         bridge_PopMatrix
#define glLoadIdentity      bridge_LoadIdentity
#define glMultMatrixf       bridge_MultMatrixf
#define glTranslatef        bridge_Translatef
#define glRotatef           bridge_Rotatef
#define glScalef            bridge_Scalef
#define glOrtho(l,r,b,t,n,f) bridge_Ortho(l,r,b,t,n,f)
#define glFrustum(l,r,b,t,n,f) bridge_Frustum(l,r,b,t,n,f)
#define glLightfv           bridge_Lightfv
#define glMaterialfv        bridge_Materialfv
#define glFogfv             bridge_Fogfv
#define glFogf              bridge_Fogf
#define glFogi              bridge_Fogi
#define glAlphaFunc         bridge_AlphaFunc
#define glTexEnvi           bridge_TexEnvi
// ... etc
#endif
```

---

## Touch Controls

### Architecture

Create a touch control system with:

1. **Virtual Joystick** (left side): Analog input for movement
2. **Action Buttons** (right side): Diamond layout for game actions
3. **System Buttons**: Pause, debug toggle, etc.

### Design Principles

- **Transparent**: Buttons should be mostly transparent so they don't obscure gameplay
- **Consistent appearance**: Set GL color state before every draw call to prevent state leakage
- **Generous hit areas**: Touch targets should be ~30% larger than visual size
- **Dead zone**: Joystick should have a ~15% dead zone to prevent accidental movement

### Implementation

```c
// Virtual joystick: circle background + movable thumb indicator
static void DrawJoystick(float cx, float cy, float radius) {
    // Subtle transparent background
    glColor4f(0.3f, 0.3f, 0.3f, 0.15f);
    DrawFilledCircle(cx, cy, radius, 32);

    // Outline
    glColor4f(0.7f, 0.7f, 0.7f, 0.4f);
    DrawCircleOutline(cx, cy, radius, 32);

    // Thumb indicator (moves with touch)
    float thumbX = cx + analogX * radius * 0.6f;
    float thumbY = cy + analogY * radius * 0.6f;
    glColor4f(0.5f, 0.5f, 0.5f, 0.35f);
    DrawFilledCircle(thumbX, thumbY, radius * 0.25f, 16);
}
```

### Input Mapping

Map touch buttons to game "needs" in your input system:

```c
case kNeed_Jump:
    downNow |= TouchControls_IsPressed(TOUCH_BUTTON_JUMP);
    break;
case kNeed_Shoot:
    downNow |= TouchControls_IsPressed(TOUCH_BUTTON_SHOOT);
    break;
// Also map joystick directions to UI navigation for menus:
case kNeed_UIUp:
    downNow |= TouchControls_IsPressed(TOUCH_BUTTON_DPAD_UP);
    break;
```

### Preventing Button Glitches

The most common cause of visual glitches in touch controls is **GL state leakage**. Always:

1. Disable `GL_CULL_FACE` during 2D overlay rendering (Y-flipped ortho changes winding)
2. Set `glColor4f()` before every single draw call
3. Set `glLineWidth()` before every outline draw
4. Disable `GL_TEXTURE_2D`, `GL_LIGHTING`, `GL_DEPTH_TEST` at the start of overlay

---

## Common Pitfalls

### 1. GL_TEXTURE Matrix Mode

If your game uses `glMatrixMode(GL_TEXTURE)` for UV animation (e.g., scrolling textures on planets, water), you **must** implement a texture matrix stack in your bridge. Otherwise texture transforms will corrupt the modelview matrix:

```c
// Without texture matrix support, this corrupts vertex positions:
glMatrixMode(GL_TEXTURE);
glTranslatef(scrollU, scrollV, 0);  // Accidentally moves geometry!
```

### 2. `GL_QUADS` Not Supported

GLES doesn't support `GL_QUADS`. Convert to triangles:

```c
// Convert 4 quad vertices to 6 triangle indices
for (int i = 0; i < quadCount; i++) {
    indices[i*6+0] = i*4+0;  indices[i*6+1] = i*4+1;  indices[i*6+2] = i*4+2;
    indices[i*6+3] = i*4+0;  indices[i*6+4] = i*4+2;  indices[i*6+5] = i*4+3;
}
```

### 3. `GL_UNSIGNED_INT` Indices

GLES 2.0 doesn't support 32-bit indices. GLES 3.0 does, but if targeting GLES 2.0, convert to `GL_UNSIGNED_SHORT`.

### 4. Separate VAOs for Different Draw Modes

Use separate VAOs for immediate mode rendering vs. vertex-array draws. Sharing a VAO causes VBO state contamination.

### 5. `glPolygonMode` Doesn't Exist

GLES has no wireframe mode. Remove or stub `glPolygonMode` calls.

### 6. Display Lists Don't Exist

GLES has no display lists. If your game uses them, convert to VBOs or just call the drawing code directly.

### 7. Asset Loading

On Android, assets are packaged inside the APK. SDL handles this transparently — `SDL_RWFromFile()` can read from the assets directory. Just make sure your Gradle config includes the data directory:

```kotlin
sourceSets {
    getByName("main") {
        assets.srcDirs("../../Data")
    }
}
```

---

## Testing & Debugging

### LogCat

Use `__android_log_print` for logging:

```c
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "MyGame", __VA_ARGS__)
```

View logs with: `adb logcat -s MyGame`

### Common GL Errors

| Error | Code | Common Cause |
|-------|------|-------------|
| `GL_INVALID_ENUM` | 0x500 | Using unsupported enum (e.g., `GL_QUADS`, `GL_LUMINANCE`) |
| `GL_INVALID_VALUE` | 0x501 | Negative size, invalid dimension |
| `GL_INVALID_OPERATION` | 0x502 | Client-side arrays with non-default VAO, state mismatch |
| `GL_OUT_OF_MEMORY` | 0x505 | Too many textures, too large VBO |

### CI/CD

Set up a GitHub Actions workflow to build the APK automatically:

```yaml
name: Android Build
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v4
      with:
        submodules: recursive
    - uses: actions/setup-java@v4
      with:
        java-version: '17'
        distribution: 'temurin'
    - name: Clone SDL3
      run: git clone --depth 1 https://github.com/libsdl-org/SDL.git extern/SDL
    - name: Build APK
      working-directory: android
      run: ./gradlew assembleDebug
    - uses: actions/upload-artifact@v4
      with:
        name: debug-apk
        path: android/app/build/outputs/apk/debug/*.apk
```

---

## Checklist

Use this checklist when porting a new game:

- [ ] **Project Setup**
  - [ ] Create `android/` directory with Gradle build files
  - [ ] Copy SDL Java bridge files
  - [ ] Create `AndroidManifest.xml`
  - [ ] Set up launcher icons
  - [ ] Configure CMakeLists.txt for Android (shared lib, NDK flags)

- [ ] **OpenGL Migration**
  - [ ] Audit all GL calls in the codebase (`grep -rn "gl[A-Z]"`)
  - [ ] Create `gles_compat.h` compatibility header
  - [ ] Implement `GLESBridge.c` with shader-based emulation
  - [ ] Handle matrix stacks (modelview, projection, texture)
  - [ ] Handle immediate mode (`glBegin`/`glEnd`)
  - [ ] Handle lighting (uniforms for light positions, colors)
  - [ ] Handle fog (linear fog via fragment shader)
  - [ ] Handle alpha test (`discard` in fragment shader)
  - [ ] Handle multi-texture (second sampler)
  - [ ] Handle texture generation (sphere mapping)
  - [ ] Convert unsupported texture formats (`GL_LUMINANCE` → `GL_R8`)
  - [ ] Convert `GL_QUADS` to triangles
  - [ ] Replace client-side arrays with streaming VBOs
  - [ ] Use separate VAOs for different draw modes

- [ ] **Input**
  - [ ] Implement virtual joystick
  - [ ] Implement action buttons
  - [ ] Map touch inputs to game controls
  - [ ] Ensure touch controls appear on all screens (menus too)
  - [ ] Test multi-touch (joystick + buttons simultaneously)

- [ ] **Testing**
  - [ ] Build for all 4 ABIs (arm64-v8a, armeabi-v7a, x86, x86_64)
  - [ ] Test on physical device
  - [ ] Check LogCat for GL errors
  - [ ] Verify all visual features work (textures, lighting, fog, particles)
  - [ ] Verify touch controls are responsive
  - [ ] Set up CI/CD for automated builds

---

## Resources

- [SDL3 Documentation](https://wiki.libsdl.org/SDL3/)
- [OpenGL ES 3.0 Reference](https://registry.khronos.org/OpenGL-Refpages/es3.0/)
- [Android NDK Guides](https://developer.android.com/ndk/guides)
- [GLES 3.0 Quick Reference Card](https://www.khronos.org/files/opengles3-quick-reference-card.pdf)
