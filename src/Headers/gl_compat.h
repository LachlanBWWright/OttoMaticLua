//
// gl_compat.h
// OpenGL compatibility layer - wraps legacy OpenGL calls for WebGL
//

#pragma once

#include <SDL3/SDL_opengl.h>

#ifdef __EMSCRIPTEN__
#include "modern_gl.h"

// Immediate mode emulation
#define glBegin(mode) ModernGL_BeginImmediateMode(mode)
#define glEnd() ModernGL_EndImmediateMode()
#define glColor3f(r,g,b) ModernGL_ImmediateColor(r,g,b,1.0f)
#define glColor4f(r,g,b,a) ModernGL_ImmediateColor(r,g,b,a)
#define glColor4fv(v) ModernGL_ImmediateColor((v)[0],(v)[1],(v)[2],(v)[3])
#define glNormal3f(x,y,z) ModernGL_ImmediateNormal(x,y,z)
#define glTexCoord2f(u,v) ModernGL_ImmediateTexCoord(u,v)
#define glVertex2f(x,y) ModernGL_ImmediateVertex(x,y,0.0f)
#define glVertex3f(x,y,z) ModernGL_ImmediateVertex(x,y,z)
#define glVertex3fv(v) ModernGL_ImmediateVertex((v)[0],(v)[1],(v)[2])

// GL_QUADS not supported in WebGL - need to convert to triangles
// This is handled in the immediate mode implementation
// For GL_QUADS: vertices 0,1,2,3 become triangles 0,1,2 and 0,2,3

#endif // __EMSCRIPTEN__
