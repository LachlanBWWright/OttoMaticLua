#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "game.h"

OGLActiveTextureProc		procptr_glActiveTexture = NULL;
OGLClientActiveTextureProc	procptr_glClientActiveTexture = NULL;

void OGL_InitFunctions(void)
{
	procptr_glActiveTexture = (OGLActiveTextureProc) SDL_GL_GetProcAddress("glActiveTexture");
	if (procptr_glActiveTexture == NULL)
		procptr_glActiveTexture = (OGLActiveTextureProc) SDL_GL_GetProcAddress("glActiveTextureARB");

	procptr_glClientActiveTexture = (OGLClientActiveTextureProc) SDL_GL_GetProcAddress("glClientActiveTexture");
	if (procptr_glClientActiveTexture == NULL)
		procptr_glClientActiveTexture = (OGLClientActiveTextureProc) SDL_GL_GetProcAddress("glClientActiveTextureARB");

	GAME_ASSERT(procptr_glActiveTexture != NULL);
	GAME_ASSERT(procptr_glClientActiveTexture != NULL);
}

void OGL_ActiveTexture(GLenum texture)
{
	procptr_glActiveTexture(texture);
}

void OGL_ClientActiveTexture(GLenum texture)
{
	procptr_glClientActiveTexture(texture);
}
