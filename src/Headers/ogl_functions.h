#pragma once

#ifdef __ANDROID__
#include "gles_compat.h"
// On Android, use glActiveTexture and glClientActiveTexture directly (they're in ES 1.1)
#define glActiveTextureARB glActiveTexture
#define glClientActiveTextureARB glClientActiveTexture
// No-op init on Android
#define OGL_InitFunctions() ((void)0)
#else
#include <SDL3/SDL_opengl.h>

extern PFNGLACTIVETEXTUREARBPROC			procptr_glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC		procptr_glClientActiveTextureARB;

#define glActiveTextureARB					procptr_glActiveTextureARB
#define glClientActiveTextureARB			procptr_glClientActiveTextureARB

void OGL_InitFunctions(void);
#endif
