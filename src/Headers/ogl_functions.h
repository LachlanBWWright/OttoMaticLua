#pragma once

#ifdef __ANDROID__
#include "gles_compat.h"
// On Android with GLES 3.0, intercept glActiveTextureARB through the bridge
// so we can track which texture unit is active for enable/disable state
#define glActiveTextureARB bridge_ActiveTexture
// glClientActiveTextureARB is handled by the bridge (redirected in gles_compat.h)
// Initialize the GLES 3.0 bridge
#define OGL_InitFunctions() GLESBridge_Init()
#else
#include <SDL3/SDL_opengl.h>

extern PFNGLACTIVETEXTUREARBPROC			procptr_glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC		procptr_glClientActiveTextureARB;

#define glActiveTextureARB					procptr_glActiveTextureARB
#define glClientActiveTextureARB			procptr_glClientActiveTextureARB

void OGL_InitFunctions(void);
#endif
