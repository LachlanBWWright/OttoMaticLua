#include "game.h"

#if defined(__NDS__) || defined(OTTO_DS_HOMEBREW)

PFNGLACTIVETEXTUREARBPROC			procptr_glActiveTextureARB			= NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC		procptr_glClientActiveTextureARB	= NULL;

void OGL_InitFunctions(void)
{
	procptr_glActiveTextureARB			= NULL;
	procptr_glClientActiveTextureARB	= NULL;
}

#else

PFNGLACTIVETEXTUREARBPROC			procptr_glActiveTextureARB			= NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC		procptr_glClientActiveTextureARB	= NULL;

void OGL_InitFunctions(void)
{
	procptr_glActiveTextureARB			= (PFNGLACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glActiveTextureARB");
	procptr_glClientActiveTextureARB	= (PFNGLCLIENTACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glClientActiveTextureARB");

	GAME_ASSERT(procptr_glActiveTextureARB);
	GAME_ASSERT(procptr_glClientActiveTextureARB);
}

#endif
