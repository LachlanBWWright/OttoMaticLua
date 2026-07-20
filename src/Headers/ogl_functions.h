#pragma once

#include <SDL3/SDL_opengl.h>

typedef void (APIENTRY *OGLActiveTextureProc)(GLenum texture);
typedef void (APIENTRY *OGLClientActiveTextureProc)(GLenum texture);

extern OGLActiveTextureProc		procptr_glActiveTexture;
extern OGLClientActiveTextureProc	procptr_glClientActiveTexture;

#ifndef GL_TEXTURE0_ARB
#define GL_TEXTURE0_ARB GL_TEXTURE0
#endif

#ifndef GL_TEXTURE1_ARB
#define GL_TEXTURE1_ARB GL_TEXTURE1
#endif

void OGL_ActiveTexture(GLenum texture);
void OGL_ClientActiveTexture(GLenum texture);

#define glActiveTextureARB(texture) OGL_ActiveTexture(texture)
#define glClientActiveTextureARB(texture) OGL_ClientActiveTexture(texture)

void OGL_InitFunctions(void);
