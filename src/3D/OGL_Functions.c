#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "game.h"

PFNGLACTIVETEXTUREARBPROC			procptr_glActiveTextureARB			= NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC		procptr_glClientActiveTextureARB	= NULL;

#ifdef __EMSCRIPTEN__
// WebGL doesn't support glClientActiveTexture because it's part of the legacy
// fixed-function pipeline. In WebGL, texture coordinate arrays are automatically
// associated with the active texture unit when using glTexCoordPointer.
// This stub function is provided for API compatibility.
static void glClientActiveTexture_stub(GLenum texture)
{
	(void)texture;  // Unused - WebGL handles this automatically
}
#endif

void OGL_InitFunctions(void)
{
	procptr_glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glActiveTextureARB");
	if (!procptr_glActiveTextureARB)
		procptr_glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glActiveTexture");

	GAME_ASSERT(procptr_glActiveTextureARB);

#ifdef __EMSCRIPTEN__
	// WebGL/Emscripten doesn't provide glClientActiveTexture, so use our stub
	procptr_glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) glClientActiveTexture_stub;
#else
	procptr_glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glClientActiveTextureARB");
	if (!procptr_glClientActiveTextureARB)
		procptr_glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glClientActiveTexture");

	GAME_ASSERT(procptr_glClientActiveTextureARB);
#endif
}
