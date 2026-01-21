# Otto Matic Android Build Guide

This document describes how to build Otto Matic for Android.

## Prerequisites

- Android Studio (recommended) or command-line tools
- Android SDK with API level 34
- Android NDK version 27.x (tested with 27.3.13750724)
- CMake 3.22.1 or higher
- JDK 17 or higher

## Build Setup

1. Clone the repository with submodules:
   ```bash
   git clone --recursive https://github.com/jorio/ottomatic.git
   cd ottomatic
   ```

2. Clone SDL3 into the extern folder (if not present):
   ```bash
   git clone --depth 1 https://github.com/libsdl-org/SDL.git extern/SDL
   ```

3. SDL Java files and launcher icons are already included in the repository.

4. Create `local.properties` in the `android` folder:
   ```properties
   sdk.dir=/path/to/android/sdk
   ```
   (Or set ANDROID_HOME environment variable)

## Building

### Using Command Line

```bash
cd android
chmod +x gradlew
./gradlew assembleDebug
```

The APK will be generated at `android/app/build/outputs/apk/debug/app-debug.apk`

### Using Android Studio

1. Open the `android` folder in Android Studio
2. Let Gradle sync complete
3. Select "Build > Build Bundle(s) / APK(s) > Build APK(s)"

## On-Screen Controls

The Android port features on-screen touch controls:

- **D-Pad** (left side): Movement control with analog support
- **Jump** (green button): Jump / Jet Pack
- **Shoot** (blue button): Fire weapon
- **Punch/Pickup** (red button): Melee attack / Pick up items
- **Prev/Next Weapon** (yellow buttons): Cycle through weapons
- **Pause** (top center): Pause game

## OpenGL ES Compatibility

The game includes an OpenGL ES 1.1 compatibility layer (`gles_compat.h`) that:

- Maps desktop OpenGL functions to ES equivalents (glOrtho → glOrthof, etc.)
- Stubs out unavailable functions (immediate mode rendering, texture generation)
- Defines missing constants for compatibility

**Current Status:**
- ✅ Touch controls render correctly (uses vertex arrays)
- ✅ Game logic works
- ⚠️ Some visual effects using immediate mode (glBegin/glEnd) are stubbed

For full game rendering, consider using [gl4es](https://github.com/ptitSeb/gl4es).

## Troubleshooting

### Build fails with "Could not resolve com.android.tools.build:gradle"
Ensure you have internet access to Google's Maven repository (dl.google.com).

### NDK version mismatch
Update the `ndkVersion` in `app/build.gradle.kts` to match your installed NDK version.

### Native library not found
Ensure the CMake build completes successfully and the shared library is generated.

## Known Limitations

- Immediate mode OpenGL (glBegin/glEnd) is stubbed - some visual effects won't render
- Touch controls are implemented and functional
- The on-screen controls currently only support landscape orientation

## CI/CD

The repository includes a GitHub Actions workflow (`AndroidBuild.yml`) that automatically builds the APK on push/PR to main branches.
