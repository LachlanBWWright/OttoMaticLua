# Otto Matic Android APK

This directory contains the Android APK split into multiple parts to fit within GitHub's file size limits (100MB).

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
copy /b OttoMatic-debug.apk.partaa+OttoMatic-debug.apk.partab+OttoMatic-debug.apk.partac+OttoMatic-debug.apk.partad OttoMatic-debug.apk
```

## Installing on Android

1. Reassemble the APK using the commands above
2. Transfer `OttoMatic-debug.apk` to your Android device
3. Enable "Install from unknown sources" in your device settings
4. Open the APK file on your device to install

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
- **Size**: ~150MB (includes game assets)

## Checksum

After reassembling, verify the APK integrity:
```bash
sha256sum OttoMatic-debug.apk
# Expected: c964fba4168aa3cb32955680db786ffa9130e60fa97bda35bdabe6ea42c40072
```
