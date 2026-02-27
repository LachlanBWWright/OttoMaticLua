#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "game.h"

PFNGLACTIVETEXTUREARBPROC			procptr_glActiveTextureARB			= NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC		procptr_glClientActiveTextureARB	= NULL;

#ifdef __EMSCRIPTEN__
// WebGL doesn't support glClientActiveTexture because it's part of the legacy
// fixed-function pipeline. The modern GL system replaces client-side vertex arrays
// with VBOs and shader-based rendering, making this function unnecessary.
// This no-op function is provided only for API compatibility with legacy code paths.
static void glClientActiveTexture_noop(GLenum texture)
{
	(void)texture;  // No-op: Modern GL uses VBOs with vertex attributes
}
#endif

void OGL_InitFunctions(void)
{
#ifdef __EMSCRIPTEN__
	// Initialize modern GL subsystem for WebAssembly
	SDL_Log("OGL_InitFunctions: Initializing ModernGL subsystem for WebAssembly...");
	extern void ModernGL_Init(void);
	ModernGL_Init();
	SDL_Log("OGL_InitFunctions: ModernGL subsystem initialized");
#endif

	procptr_glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glActiveTextureARB");
	if (!procptr_glActiveTextureARB)
		procptr_glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glActiveTexture");

	GAME_ASSERT(procptr_glActiveTextureARB);
#ifdef __EMSCRIPTEN__
	SDL_Log("OGL_InitFunctions: glActiveTexture resolved successfully");
#endif

#ifdef __EMSCRIPTEN__
	// WebGL/Emscripten doesn't provide glClientActiveTexture
	// Modern GL rendering path doesn't need this function
	procptr_glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) glClientActiveTexture_noop;
	SDL_Log("OGL_InitFunctions: Using no-op glClientActiveTexture for WebGL");
#else
	procptr_glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glClientActiveTextureARB");
	if (!procptr_glClientActiveTextureARB)
		procptr_glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glClientActiveTexture");

	GAME_ASSERT(procptr_glClientActiveTextureARB);
#endif
}
