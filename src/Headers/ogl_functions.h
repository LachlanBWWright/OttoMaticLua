#pragma once

#include "graphics_api.h"

#if !defined(__NDS__) && !defined(OTTO_DS_HOMEBREW)
	#include <SDL3/SDL_opengl.h>
#endif

extern PFNGLACTIVETEXTUREARBPROC			procptr_glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC		procptr_glClientActiveTextureARB;

#define glActiveTextureARB					procptr_glActiveTextureARB
#define glClientActiveTextureARB			procptr_glClientActiveTextureARB

void OGL_InitFunctions(void);
