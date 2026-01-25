#include <SDL3/SDL.h>
#include "game.h"

#ifndef __ANDROID__
// Desktop OpenGL only
#include <SDL3/SDL_opengl.h>

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
