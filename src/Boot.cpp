// OTTO MATIC ENTRY POINT
// (C) 2025 Iliyas Jorio
// This file is part of Otto Matic. https://github.com/jorio/ottomatic

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Pomme.h"
#include "PommeInit.h"
#include "PommeFiles.h"

#ifdef __ANDROID__
#include <android/log.h>
#include <sys/stat.h>
#include <errno.h>
#include <cstdio>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "OttoMatic", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "OttoMatic", __VA_ARGS__)
#include "android_assets.h"
#else
#define LOGI(...) SDL_Log(__VA_ARGS__)
#define LOGE(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#endif

extern "C"
{
	#include "game.h"

	SDL_Window* gSDLWindow = nullptr;
	FSSpec gDataSpec;
	int gCurrentAntialiasingLevel;
}

#ifdef __ANDROID__
// Helper function to create directories recursively
static bool CreateDirectoryRecursive(const std::string& path)
{
	size_t pos = 0;
	std::string dir;
	
	if (path[0] == '/')
		pos = 1;
	
	while ((pos = path.find('/', pos)) != std::string::npos)
	{
		dir = path.substr(0, pos++);
		if (dir.empty())
			continue;
		if (mkdir(dir.c_str(), 0755) != 0 && errno != EEXIST)
			return false;
	}
	
	if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST)
		return false;
	
	return true;
}

// Helper function to check if a file exists
static bool FileExists(const std::string& path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0;
}

// Extract a single asset file from APK to internal storage
static bool ExtractAsset(const std::string& assetPath, const std::string& destPath)
{
	// Open asset from APK using SDL's IOFromFile which reads from APK assets on Android
	SDL_IOStream* src = SDL_IOFromFile(assetPath.c_str(), "rb");
	if (!src)
	{
		LOGE("Failed to open asset: %s - %s", assetPath.c_str(), SDL_GetError());
		return false;
	}
	
	// Get asset size
	Sint64 size = SDL_GetIOSize(src);
	if (size < 0)
	{
		LOGE("Failed to get asset size: %s", assetPath.c_str());
		SDL_CloseIO(src);
		return false;
	}
	
	// Read asset data
	void* data = SDL_malloc((size_t)size);
	if (!data)
	{
		LOGE("Failed to allocate memory for asset: %s (%lld bytes)", assetPath.c_str(), (long long)size);
		SDL_CloseIO(src);
		return false;
	}
	
	size_t bytesRead = SDL_ReadIO(src, data, (size_t)size);
	SDL_CloseIO(src);
	
	if (bytesRead != (size_t)size)
	{
		LOGE("Failed to read asset: %s (read %zu of %lld bytes)", assetPath.c_str(), bytesRead, (long long)size);
		SDL_free(data);
		return false;
	}
	
	// Create destination directory if needed
	std::string destDir = destPath.substr(0, destPath.rfind('/'));
	if (!CreateDirectoryRecursive(destDir))
	{
		LOGE("Failed to create directory: %s", destDir.c_str());
		SDL_free(data);
		return false;
	}
	
	// Write to destination
	FILE* dest = fopen(destPath.c_str(), "wb");
	if (!dest)
	{
		LOGE("Failed to create file: %s - %s", destPath.c_str(), strerror(errno));
		SDL_free(data);
		return false;
	}
	
	size_t written = fwrite(data, 1, (size_t)size, dest);
	fclose(dest);
	SDL_free(data);
	
	if (written != (size_t)size)
	{
		LOGE("Failed to write file: %s (wrote %zu of %lld bytes)", destPath.c_str(), written, (long long)size);
		return false;
	}
	
	return true;
}

static const char* ASSET_MARKER = ".otto_assets_v1";

// Extract all assets from APK to internal storage
static bool ExtractAllAssets(const std::string& internalPath)
{
	std::string markerPath = internalPath + "/" + ASSET_MARKER;
	
	// Check if assets are already extracted
	if (FileExists(markerPath))
	{
		LOGI("Assets already extracted (found marker), skipping extraction");
		return true;
	}
	
	LOGI("First run detected - extracting %d assets to internal storage...", (int)NUM_GAME_ASSETS);
	LOGI("Destination: %s", internalPath.c_str());
	
	int extracted = 0;
	int failed = 0;
	
	for (size_t i = 0; i < NUM_GAME_ASSETS; i++)
	{
		const char* assetPath = ALL_GAME_ASSETS[i];
		if (!assetPath) break;
		
		std::string destPath = internalPath + "/" + assetPath;
		
		// Skip if already exists
		if (FileExists(destPath))
		{
			extracted++;
			continue;
		}
		
		// Log progress every 50 files
		if (i % 50 == 0)
		{
			LOGI("Extracting... %zu/%d", i, (int)NUM_GAME_ASSETS);
		}
		
		if (ExtractAsset(assetPath, destPath))
		{
			extracted++;
		}
		else
		{
			failed++;
			// Continue with other files even if some fail
		}
	}
	
	LOGI("Asset extraction complete: %d extracted, %d failed", extracted, failed);
	
	// Create marker file to indicate extraction is complete
	if (failed == 0)
	{
		FILE* marker = fopen(markerPath.c_str(), "w");
		if (marker)
		{
			fprintf(marker, "version=1\nextracted=%d\n", extracted);
			fclose(marker);
			LOGI("Created extraction marker file");
		}
	}
	else
	{
		LOGE("Some assets failed to extract - not creating marker file");
	}
	
	return failed == 0;
}
#endif // __ANDROID__

static fs::path FindGameData(const char* executablePath)
{
	fs::path dataPath;

	int attemptNum = 0;
	(void)attemptNum; // Suppress unused variable warning on Android

#ifdef __ANDROID__
	// On Android, get the internal storage path where we can write files
	const char* internalPath = SDL_GetAndroidInternalStoragePath();
	if (!internalPath)
	{
		LOGE("Failed to get Android internal storage path!");
		throw std::runtime_error("Failed to get Android internal storage path");
	}
	
	LOGI("Android internal storage path: %s", internalPath);
	dataPath = internalPath;
	
	// Extract assets from APK to internal storage
	// This is needed because Pomme uses std::fstream which can't read from APK
	ExtractAllAssets(internalPath);
	
	// Set data spec to point to internal storage
	gDataSpec = Pomme::Files::HostPathToFSSpec(dataPath / "System");
	
	return dataPath;
#else

#if !(__APPLE__)
	attemptNum++;		// skip macOS special case #0
#endif

	if (!executablePath)
		attemptNum = 2;

tryAgain:
	switch (attemptNum)
	{
		case 0:			// special case for macOS app bundles
			dataPath = executablePath;
			dataPath = dataPath.parent_path().parent_path() / "Resources";
			break;

		case 1:
			dataPath = executablePath;
			dataPath = dataPath.parent_path() / "Data";
			break;

		case 2:
			dataPath = "Data";
			break;

		default:
			throw std::runtime_error("Couldn't find the Data folder.");
	}

	attemptNum++;

	dataPath = dataPath.lexically_normal();

	// Set data spec -- Lets the game know where to find its asset files
	gDataSpec = Pomme::Files::HostPathToFSSpec(dataPath / "System");

	FSSpec someDataFileSpec;
	OSErr iErr = FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, ":System:gamecontrollerdb.txt", &someDataFileSpec);
	if (iErr)
	{
		goto tryAgain;
	}

	return dataPath;
#endif // !__ANDROID__
}

static void Boot(int argc, char** argv)
{
	SDL_SetAppMetadata(GAME_FULL_NAME, GAME_VERSION, GAME_IDENTIFIER);
#if _DEBUG
	SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
#else
	SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
#endif

	LOGI("Otto Matic starting up...");

#ifdef __ANDROID__
	// On Android, we need to set the HOME environment variable BEFORE initializing Pomme
	// The Pomme library's FindFolder() uses HOME for the preferences folder
	const char* androidInternalPath = SDL_GetAndroidInternalStoragePath();
	if (androidInternalPath)
	{
		// Set HOME to internal storage so Pomme can find/create preferences folder
		setenv("HOME", androidInternalPath, 1);
		LOGI("Set HOME environment variable to: %s", androidInternalPath);
		
		// Also create the .config directory that Pomme expects on Linux/Android
		std::string configDir = std::string(androidInternalPath) + "/.config";
		CreateDirectoryRecursive(configDir);
		LOGI("Created config directory: %s", configDir.c_str());
	}
	else
	{
		LOGE("WARNING: Could not get Android internal storage path for HOME");
	}
#endif

	// Start our "machine"
	Pomme::Init();

	// Find path to game data folder
	const char* executablePath = argc > 0 ? argv[0] : NULL;
	fs::path dataPath = FindGameData(executablePath);

	// Load game prefs before starting
	LoadPrefs();

retryVideo:
	// Initialize SDL video subsystem
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		throw std::runtime_error("Couldn't initialize SDL video subsystem.");
	}

#ifdef __ANDROID__
	// On Android, use OpenGL ES 1.1 for fixed-function pipeline support
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	
	// Android windows are fullscreen by default
	gSDLWindow = SDL_CreateWindow(
		GAME_FULL_NAME " " GAME_VERSION, 0, 0,
		SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
	
	LOGI("Created Android window with OpenGL ES 1.1");
#else
	// Create window
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	gCurrentAntialiasingLevel = gGamePrefs.antialiasingLevel;
	if (gCurrentAntialiasingLevel != 0)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1 << gCurrentAntialiasingLevel);
	}

	gSDLWindow = SDL_CreateWindow(
		GAME_FULL_NAME " " GAME_VERSION, 640, 480,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
#endif

	if (!gSDLWindow)
	{
#ifndef __ANDROID__
		if (gCurrentAntialiasingLevel != 0)
		{
			SDL_Log("Couldn't create SDL window with the requested MSAA level. Retrying without MSAA...");

			// retry without MSAA
			gGamePrefs.antialiasingLevel = 0;
			SDL_QuitSubSystem(SDL_INIT_VIDEO);
			goto retryVideo;
		}
		else
#endif
		{
			throw std::runtime_error("Couldn't create SDL window.");
		}
	}

	// Init gamepad subsystem
	SDL_Init(SDL_INIT_GAMEPAD);
#ifndef __ANDROID__
	auto gamecontrollerdbPath8 = (dataPath / "System" / "gamecontrollerdb.txt").u8string();
	if (-1 == SDL_AddGamepadMappingsFromFile((const char*)gamecontrollerdbPath8.c_str()))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, GAME_FULL_NAME, "Couldn't load gamecontrollerdb.txt!", gSDLWindow);
	}
#endif
}

static void Shutdown()
{
#ifndef __ANDROID__
	// Always restore the user's mouse acceleration before exiting.
	SetMacLinearMouse(false);
#endif

	Pomme::Shutdown();

	if (gSDLWindow)
	{
		SDL_DestroyWindow(gSDLWindow);
		gSDLWindow = NULL;
	}

	SDL_Quit();
}

int main(int argc, char** argv)
{
	bool success = true;
	std::string uncaught = "";

	LOGI("main() called with argc=%d", argc);

	try
	{
		Boot(argc, argv);
		GameMain();
	}
	catch (Pomme::QuitRequest&)
	{
		// no-op, the game may throw this exception to shut us down cleanly
	}
#if !(_DEBUG)
	// In release builds, catch anything that might be thrown by GameMain
	// so we can show an error dialog to the user.
	catch (std::exception& ex)		// Last-resort catch
	{
		success = false;
		uncaught = ex.what();
	}
	catch (...)						// Last-resort catch
	{
		success = false;
		uncaught = "unknown";
	}
#endif

	Shutdown();

	if (!success)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Uncaught exception: %s", uncaught.c_str());
		SDL_ShowSimpleMessageBox(0, GAME_FULL_NAME, uncaught.c_str(), nullptr);
	}

	return success ? 0 : 1;
}
