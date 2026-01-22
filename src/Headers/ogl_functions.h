#pragma once

#ifdef __ANDROID__
#include "gles_compat.h"
// On Android with ES 3.0, use glActiveTexture directly
// glClientActiveTexture doesn't exist in ES 2.0+ (no fixed-function texture units)
#define OGL_InitFunctions() ((void)0)
#else
#include <SDL3/SDL_opengl.h>

extern PFNGLACTIVETEXTUREARBPROC			procptr_glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC		procptr_glClientActiveTextureARB;

#define glActiveTextureARB					procptr_glActiveTextureARB
#define glClientActiveTextureARB			procptr_glClientActiveTextureARB

void OGL_InitFunctions(void);
#endif
