#pragma once

#ifdef NDS
// NDS uses direct function calls via nds_gl_compat.h, no function pointers needed
#define glActiveTextureARB          NDS_glActiveTexture
#define glClientActiveTextureARB    NDS_glClientActiveTexture

static inline void OGL_InitFunctions(void) {}

#else

#include <SDL3/SDL_opengl.h>

extern PFNGLACTIVETEXTUREARBPROC			procptr_glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC		procptr_glClientActiveTextureARB;

#define glActiveTextureARB					procptr_glActiveTextureARB
#define glClientActiveTextureARB			procptr_glClientActiveTextureARB

void OGL_InitFunctions(void);

#endif // NDS
