// ANDROID LEVEL SELECT DIALOG
// Calls back into Java via JNI to show a native Android UI dialog
// that lets the player choose a level or upload custom terrain files.

#ifdef __ANDROID__

#include "game.h"
#include <jni.h>
#include <SDL3/SDL.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "OttoMatic", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "OttoMatic", __VA_ARGS__)

/********************* ANDROID SHOW LEVEL SELECT DIALOG **********************/
//
// Shows an Android native dialog that lets the user pick a level (0-9)
// or load custom terrain files.  Blocks until the user dismisses the dialog.
// Returns the selected level number (0-based), or -1 if cancelled.
//

int Android_ShowLevelSelectDialog(void)
{
	JNIEnv *env = (JNIEnv *)SDL_GetAndroidJNIEnv();
	if (!env)
	{
		LOGE("Android_ShowLevelSelectDialog: SDL_GetAndroidJNIEnv returned NULL");
		return -1;
	}

	jobject activity = (jobject)SDL_GetAndroidActivity();
	if (!activity)
	{
		LOGE("Android_ShowLevelSelectDialog: SDL_GetAndroidActivity returned NULL");
		return -1;
	}

	jclass cls = (*env)->GetObjectClass(env, activity);
	if (!cls)
	{
		LOGE("Android_ShowLevelSelectDialog: GetObjectClass failed");
		(*env)->DeleteLocalRef(env, activity);
		return -1;
	}

	jmethodID method = (*env)->GetMethodID(env, cls, "showLevelSelectDialog", "()I");
	if (!method)
	{
		LOGE("Android_ShowLevelSelectDialog: GetMethodID failed (showLevelSelectDialog not found)");
		(*env)->DeleteLocalRef(env, cls);
		(*env)->DeleteLocalRef(env, activity);
		return -1;
	}

	jint result = (*env)->CallIntMethod(env, activity, method);

	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, activity);

	return (int)result;
}

#endif // __ANDROID__
