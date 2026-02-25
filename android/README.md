# Otto Matic Android Build Guide

This document describes how to build Otto Matic for Android.

## Pre-built APK

A pre-built APK is available in the `releases/` folder. Due to GitHub's file size limit, it's split into parts:

```bash
cd releases
cat OttoMatic-debug.apk.part* > OttoMatic-debug.apk
# Install: adb install OttoMatic-debug.apk
```

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
- Converts texture formats (GL_BGRA_EXT, 16-bit packed) to supported formats
- Converts 32-bit indices to 16-bit for glDrawElements (ES 1.1 limitation)
- Stubs out unavailable functions (immediate mode rendering, texture generation)
- Defines missing constants for compatibility

**Current Status:**
- ✅ Textures load correctly (format conversion working)
- ✅ 3D geometry renders (index conversion working)
- ✅ Touch controls render correctly (uses vertex arrays)
- ✅ Game logic works
- ⚠️ Some visual effects using immediate mode (glBegin/glEnd) are stubbed

For full game rendering, consider using [gl4es](https://github.com/ptitSeb/gl4es).

## First Run

On first launch, the app extracts ~200MB of game assets from the APK to internal storage. This is required because the Pomme library uses file I/O that cannot read directly from APK assets. Subsequent launches will be fast.

## Troubleshooting

### Build fails with "Could not resolve com.android.tools.build:gradle"
Ensure you have internet access to Google's Maven repository (dl.google.com).

### NDK version mismatch
Update the `ndkVersion` in `app/build.gradle.kts` to match your installed NDK version.

### Native library not found
Ensure the CMake build completes successfully and the shared library is generated.

### App crashes on startup
- Check logcat for "SDL_main" errors - the entry point should be found
- Check for "preferences folder" warnings - HOME environment should be set
- Check for GL errors (0x502 = GL_INVALID_OPERATION) - texture formats should be converted

## Known Limitations

- Immediate mode OpenGL (glBegin/glEnd) is stubbed - some visual effects won't render
- First run extracts ~200MB of assets (subsequent runs are fast)
- Touch controls currently only support landscape orientation

## CI/CD

The repository includes a GitHub Actions workflow (`AndroidBuild.yml`) that automatically builds the APK on push/PR to main branches.
