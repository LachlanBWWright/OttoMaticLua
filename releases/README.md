# Otto Matic Android APK

This directory contains the Android APK split into two parts to fit within GitHub's file size limits (100MB).

The APK was built with GLES 3.0 shader-based rendering, including multi-texture blending and sphere map reflections.

## Reassembling the APK

### On Linux/macOS
```bash
cd releases
cat OttoMatic-debug.apk.part* > OttoMatic-debug.apk
```

### On Windows (PowerShell)
```powershell
cd releases
Get-Content OttoMatic-debug.apk.part* -Encoding Byte -ReadCount 0 | Set-Content OttoMatic-debug.apk -Encoding Byte
```

### On Windows (Command Prompt)
```cmd
cd releases
copy /b OttoMatic-debug.apk.part00+OttoMatic-debug.apk.part01 OttoMatic-debug.apk
```

## Installing on Android

### Using ADB (recommended)
```bash
# Reassemble the APK first
cat OttoMatic-debug.apk.part* > OttoMatic-debug.apk

# Install via ADB
adb install OttoMatic-debug.apk
```

### Manual installation
1. Reassemble the APK using the commands above
2. Transfer `OttoMatic-debug.apk` to your Android device
3. Enable "Install from unknown sources" in your device settings
4. Open the APK file on your device to install

## CI/CD Builds

The GitHub Actions CI workflow automatically builds and uploads APK artifacts on every push.
To download the latest APK:
1. Go to the **Actions** tab
2. Click on the latest **Android Build** workflow run
3. Download the **otto-matic-debug-apk** artifact

## Building from Source

Alternatively, you can build the APK yourself:

```bash
# Clone the repository with submodules
git clone --recursive https://github.com/LachlanBWWright/OttoMaticLua.git
cd OttoMaticLua

# Clone SDL3
git clone --depth 1 https://github.com/libsdl-org/SDL.git extern/SDL

# Build the APK
cd android
./gradlew assembleDebug
```

The APK will be at: `android/app/build/outputs/apk/debug/app-debug.apk`

## APK Details

- **Version**: 4.0.2
- **Package**: io.jor.ottomatic
- **Min SDK**: 24 (Android 7.0)
- **Target SDK**: 34
- **Architectures**: armeabi-v7a, arm64-v8a, x86, x86_64
- **OpenGL**: GLES 3.0 with shader-based fixed-function emulation
- **Size**: ~151MB (includes game assets for all architectures)

## Verify Checksums

After reassembling, verify the APK integrity:
```bash
cd releases
sha256sum -c checksums.sha256
```
