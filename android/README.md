# Otto Matic Android Build Guide

This document describes how to build Otto Matic for Android.

## Prerequisites

- Android Studio (recommended) or command-line tools
- Android SDK with API level 34
- Android NDK (version 25+)
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

3. Copy SDL's Android Java files:
   ```bash
   mkdir -p android/app/src/main/java/org/libsdl/app
   cp extern/SDL/android-project/app/src/main/java/org/libsdl/app/*.java android/app/src/main/java/org/libsdl/app/
   ```

4. Copy launcher icons (or create your own):
   ```bash
   for dpi in mdpi hdpi xhdpi xxhdpi xxxhdpi; do
     mkdir -p android/app/src/main/res/mipmap-$dpi
     cp extern/SDL/android-project/app/src/main/res/mipmap-$dpi/ic_launcher.png android/app/src/main/res/mipmap-$dpi/
   done
   ```

5. Create `local.properties` in the `android` folder:
   ```properties
   sdk.dir=/path/to/android/sdk
   ndk.dir=/path/to/android/ndk
   ```
   (Or set ANDROID_HOME and ANDROID_NDK environment variables)

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

## Troubleshooting

### Build fails with "Could not resolve com.android.tools.build:gradle"
Ensure you have internet access to Google's Maven repository (dl.google.com).

### Native library not found
Ensure the CMake build completes successfully and the shared library is generated.

### OpenGL ES errors
The game's original codebase uses OpenGL immediate mode (glBegin/glEnd) which is not 
available in OpenGL ES. The touch controls overlay uses vertex arrays which are 
OpenGL ES 1.1 compatible. For the full game rendering to work on Android, one of 
these approaches may be needed:
- Use a compatibility layer like gl4es (https://github.com/ptitSeb/gl4es)
- Modify the game's rendering code to use vertex arrays instead of immediate mode

## Known Limitations

- The game rendering code uses OpenGL immediate mode which requires conversion 
  for OpenGL ES compatibility
- Touch controls are implemented but the full game may require a GL compatibility layer
- The on-screen controls currently only support landscape orientation

## CI/CD

The repository includes a GitHub Actions workflow (`AndroidBuild.yml`) that automatically builds the APK on push/PR to main branches.
