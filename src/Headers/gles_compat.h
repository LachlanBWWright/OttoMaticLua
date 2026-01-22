// OPENGL ES COMPATIBILITY HEADER FOR ANDROID
// Provides compatibility shims for desktop OpenGL functions
// that don't exist in OpenGL ES 1.1

#pragma once

#ifdef __ANDROID__

#include <GLES/gl.h>
#include <GLES/glext.h>

// ============================================================================
// Function mappings (desktop to ES)
// ============================================================================

#define glOrtho(left,right,bottom,top,zNear,zFar) glOrthof((float)(left),(float)(right),(float)(bottom),(float)(top),(float)(zNear),(float)(zFar))
#define glFrustum(left,right,bottom,top,zNear,zFar) glFrustumf((float)(left),(float)(right),(float)(bottom),(float)(top),(float)(zNear),(float)(zFar))
#define glClearDepth(depth) glClearDepthf((float)(depth))
#define glDepthRange(zNear,zFar) glDepthRangef((float)(zNear),(float)(zFar))

// ============================================================================
// Stubbed functions (not available in ES)
// ============================================================================

#define glColorMaterial(face, mode) ((void)0)
#define glFogi(pname, param) ((void)0)
#define glFogiv(pname, params) ((void)0)
#define glPolygonMode(face, mode) ((void)0)
#define glTexGeni(coord, pname, param) ((void)0)
#define glTexGenf(coord, pname, param) ((void)0)
#define glTexGenfv(coord, pname, params) ((void)0)
#define glLightModeli(pname, param) glLightModelf(pname, (GLfloat)(param))

// ============================================================================
// Immediate mode emulation (stubbed - no actual rendering)
// In a full implementation, these would accumulate vertices and draw on glEnd
// ============================================================================

#define glBegin(mode) ((void)0)
#define glEnd() ((void)0)
#define glVertex2f(x, y) ((void)0)
#define glVertex2i(x, y) ((void)0)
#define glVertex3f(x, y, z) ((void)0)
#define glVertex3fv(v) ((void)0)
#define glVertex3d(x, y, z) ((void)0)
#define glVertex4f(x, y, z, w) ((void)0)
#define glTexCoord2f(s, t) ((void)0)
#define glTexCoord2d(s, t) ((void)0)
#define glTexCoord2i(s, t) ((void)0)
#define glNormal3f(nx, ny, nz) ((void)0)
#define glNormal3fv(v) ((void)0)

// ============================================================================
// Helper macros for functions not in ES (use macros to avoid conflicts)
// ============================================================================

#define glColor4fv(v) glColor4f((v)[0], (v)[1], (v)[2], (v)[3])
#define glColor3fv(v) glColor4f((v)[0], (v)[1], (v)[2], 1.0f)
#define glColor3f(r, g, b) glColor4f(r, g, b, 1.0f)
// glTexCoord2fv is stubbed out since it's only used in immediate mode
#define glTexCoord2fv(v) ((void)0)

// ============================================================================
// Constants not in OpenGL ES
// ============================================================================

#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif
#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_QUAD_STRIP
#define GL_QUAD_STRIP 0x0008
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif
#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#endif
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif
#ifndef GL_CLAMP
#define GL_CLAMP GL_CLAMP_TO_EDGE
#endif
#ifndef GL_RGB5_A1
#define GL_RGB5_A1 0x8057
#endif
#ifndef GL_DOUBLE
#define GL_DOUBLE GL_FLOAT
#endif

// Texture coordinate generation constants
#ifndef GL_S
#define GL_S 0x2000
#endif
#ifndef GL_T
#define GL_T 0x2001
#endif
#ifndef GL_R
#define GL_R 0x2002
#endif
#ifndef GL_Q
#define GL_Q 0x2003
#endif
#ifndef GL_TEXTURE_GEN_MODE
#define GL_TEXTURE_GEN_MODE 0x2500
#endif
#ifndef GL_TEXTURE_GEN_S
#define GL_TEXTURE_GEN_S 0x0C60
#endif
#ifndef GL_TEXTURE_GEN_T
#define GL_TEXTURE_GEN_T 0x0C61
#endif
#ifndef GL_SPHERE_MAP
#define GL_SPHERE_MAP 0x2402
#endif

// Texture environment constants
#ifndef GL_TEXTURE0_ARB
#define GL_TEXTURE0_ARB GL_TEXTURE0
#endif
#ifndef GL_TEXTURE1_ARB
#define GL_TEXTURE1_ARB GL_TEXTURE1
#endif

// ============================================================================
// Extension function pointer types (for compatibility with extension loading code)
// ============================================================================

typedef void (*PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (*PFNGLACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (*PFNGLMULTITEXCOORD2FARBPROC)(GLenum target, GLfloat s, GLfloat t);
typedef void (*PFNGLMULTITEXCOORD2FVARBPROC)(GLenum target, const GLfloat *v);

#endif // __ANDROID__
