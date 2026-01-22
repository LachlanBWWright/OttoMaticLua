// OPENGL ES COMPATIBILITY HEADER FOR ANDROID
// Provides compatibility shims for desktop OpenGL functions
// Uses OpenGL ES 3.0 with ES 2.0/1.1 compatibility extensions

#pragma once

#ifdef __ANDROID__

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdlib.h>
#include <android/log.h>

#define GLES_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GLES_Compat", __VA_ARGS__)
#define GLES_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "GLES_Compat", __VA_ARGS__)

// ============================================================================
// Function mappings (desktop to ES)
// OpenGL ES 3.0 uses float versions of these functions
// ============================================================================

#define glOrtho(left,right,bottom,top,zNear,zFar) _gles_Ortho((float)(left),(float)(right),(float)(bottom),(float)(top),(float)(zNear),(float)(zFar))
#define glFrustum(left,right,bottom,top,zNear,zFar) _gles_Frustum((float)(left),(float)(right),(float)(bottom),(float)(top),(float)(zNear),(float)(zFar))
#define glClearDepth(depth) glClearDepthf((float)(depth))
#define glDepthRange(zNear,zFar) glDepthRangef((float)(zNear),(float)(zFar))

// OpenGL ES doesn't have glOrtho/glFrustum - we need to manually set the matrices
// These are stubs since the game uses fixed-function pipeline which ES doesn't support
static inline void _gles_Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    // In ES 3.0, there's no matrix stack. This is a no-op since we'd need shaders.
    (void)left; (void)right; (void)bottom; (void)top; (void)zNear; (void)zFar;
}

static inline void _gles_Frustum(float left, float right, float bottom, float top, float zNear, float zFar)
{
    // In ES 3.0, there's no matrix stack. This is a no-op since we'd need shaders.
    (void)left; (void)right; (void)bottom; (void)top; (void)zNear; (void)zFar;
}

// ============================================================================
// Fixed-function pipeline emulation stubs
// OpenGL ES 2.0+ doesn't have the fixed-function pipeline
// ============================================================================

// Matrix stack operations (stubbed - ES uses shaders)
#define glMatrixMode(mode) ((void)0)
#define glLoadIdentity() ((void)0)
#define glPushMatrix() ((void)0)
#define glPopMatrix() ((void)0)
#define glLoadMatrixf(m) ((void)0)
#define glMultMatrixf(m) ((void)0)
#define glTranslatef(x,y,z) ((void)0)
#define glRotatef(angle,x,y,z) ((void)0)
#define glScalef(x,y,z) ((void)0)

// glGetFloatv wrapper to handle fixed-function queries
static inline void _gles_GetFloatv(GLenum pname, GLfloat *params)
{
    // For matrix queries that don't exist in ES, return identity matrix
    if (pname == 0x0BA7 /* GL_PROJECTION_MATRIX */ || pname == 0x0BA6 /* GL_MODELVIEW_MATRIX */)
    {
        // Return identity matrix
        for (int i = 0; i < 16; i++)
            params[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        return;
    }
    // For current color, return white
    if (pname == 0x0B00 /* GL_CURRENT_COLOR */)
    {
        params[0] = params[1] = params[2] = params[3] = 1.0f;
        return;
    }
    // For other queries, call the real glGetFloatv
    glGetFloatv(pname, params);
}
#define glGetFloatv(pname, params) _gles_GetFloatv(pname, params)

// Lighting (stubbed - ES uses shaders)
#define glLightfv(light,pname,params) ((void)0)
#define glLightf(light,pname,param) ((void)0)
#define glLightModelfv(pname,params) ((void)0)
#define glLightModelf(pname,param) ((void)0)
#define glLightModeli(pname,param) ((void)0)
#define glMaterialfv(face,pname,params) ((void)0)
#define glMaterialf(face,pname,param) ((void)0)
#define glColorMaterial(face,mode) ((void)0)
#define glShadeModel(mode) ((void)0)

// Fog (stubbed - ES uses shaders)
#define glFogi(pname,param) ((void)0)
#define glFogiv(pname,params) ((void)0)
#define glFogf(pname,param) ((void)0)
#define glFogfv(pname,params) ((void)0)

// Texture environment (stubbed - ES uses shaders)
#define glTexEnvi(target,pname,param) ((void)0)
#define glTexEnvf(target,pname,param) ((void)0)
#define glTexEnvfv(target,pname,params) ((void)0)

// Texture coordinate generation (stubbed - ES uses shaders)
#define glTexGeni(coord,pname,param) ((void)0)
#define glTexGenf(coord,pname,param) ((void)0)
#define glTexGenfv(coord,pname,params) ((void)0)

// Alpha testing (use fragment shaders in ES)
#define glAlphaFunc(func,ref) ((void)0)

// Display lists (not in ES)
#define glGenLists(range) (0)
#define glNewList(list,mode) ((void)0)
#define glEndList() ((void)0)
#define glCallList(list) ((void)0)
#define glDeleteLists(list,range) ((void)0)

// Polygon mode (ES only supports filled polygons)
#define glPolygonMode(face,mode) ((void)0)

// glHint wrapper to ignore fixed-function hints
static inline void _gles_Hint(GLenum target, GLenum mode)
{
    // Skip hints that don't exist in ES 3.0
    if (target == 0x0C54 /* GL_FOG_HINT */)
        return;
    glHint(target, mode);
}
#define glHint(target, mode) _gles_Hint(target, mode)

// ============================================================================
// Immediate mode stubs (not in ES)
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
#define glTexCoord2fv(v) ((void)0)
#define glNormal3f(nx, ny, nz) ((void)0)
#define glNormal3fv(v) ((void)0)
#define glColor3f(r, g, b) ((void)0)
#define glColor3fv(v) ((void)0)
#define glColor4f(r, g, b, a) ((void)0)
#define glColor4fv(v) ((void)0)
#define glColor4ub(r, g, b, a) ((void)0)

// ============================================================================
// Vertex arrays (ES 3.0 uses VAOs, but we need to provide compatibility)
// ============================================================================

// These are provided by GLES2/3 but with different semantics
// Stub them to avoid conflicts
#define glVertexPointer(size,type,stride,pointer) ((void)0)
#define glTexCoordPointer(size,type,stride,pointer) ((void)0)
#define glNormalPointer(type,stride,pointer) ((void)0)
#define glColorPointer(size,type,stride,pointer) ((void)0)
#define glEnableClientState(array) ((void)0)
#define glDisableClientState(array) ((void)0)

// ============================================================================
// Constants not in OpenGL ES 3.0
// ============================================================================

// Matrix modes (stubbed)
#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif
#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif
#ifndef GL_TEXTURE
#define GL_TEXTURE 0x1702
#endif

// Polygon modes
#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif
#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif

// Primitive types not in ES
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_QUAD_STRIP
#define GL_QUAD_STRIP 0x0008
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif

// Lighting constants
#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif
#ifndef GL_LIGHT1
#define GL_LIGHT1 0x4001
#endif
#ifndef GL_LIGHT2
#define GL_LIGHT2 0x4002
#endif
#ifndef GL_AMBIENT
#define GL_AMBIENT 0x1200
#endif
#ifndef GL_DIFFUSE
#define GL_DIFFUSE 0x1201
#endif
#ifndef GL_SPECULAR
#define GL_SPECULAR 0x1202
#endif
#ifndef GL_POSITION
#define GL_POSITION 0x1203
#endif
#ifndef GL_SHININESS
#define GL_SHININESS 0x1601
#endif
#ifndef GL_EMISSION
#define GL_EMISSION 0x1600
#endif
#ifndef GL_LIGHT_MODEL_AMBIENT
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#endif
#ifndef GL_LIGHT_MODEL_TWO_SIDE
#define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#endif
#ifndef GL_COLOR_MATERIAL
#define GL_COLOR_MATERIAL 0x0B57
#endif
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif
#ifndef GL_NORMALIZE
#define GL_NORMALIZE 0x0BA1
#endif

// Fog constants
#ifndef GL_FOG
#define GL_FOG 0x0B60
#endif
#ifndef GL_FOG_MODE
#define GL_FOG_MODE 0x0B65
#endif
#ifndef GL_FOG_DENSITY
#define GL_FOG_DENSITY 0x0B62
#endif
#ifndef GL_FOG_START
#define GL_FOG_START 0x0B63
#endif
#ifndef GL_FOG_END
#define GL_FOG_END 0x0B64
#endif
#ifndef GL_FOG_COLOR
#define GL_FOG_COLOR 0x0B66
#endif
#ifndef GL_FOG_HINT
#define GL_FOG_HINT 0x0C54
#endif
#ifndef GL_EXP
#define GL_EXP 0x0800
#endif
#ifndef GL_EXP2
#define GL_EXP2 0x0801
#endif

// State query constants
#ifndef GL_PROJECTION_MATRIX
#define GL_PROJECTION_MATRIX 0x0BA7
#endif
#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX 0x0BA6
#endif
#ifndef GL_CURRENT_COLOR
#define GL_CURRENT_COLOR 0x0B00
#endif
#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 0x803A
#endif

// Texture constants
#ifndef GL_TEXTURE_ENV
#define GL_TEXTURE_ENV 0x2300
#endif
#ifndef GL_TEXTURE_ENV_MODE
#define GL_TEXTURE_ENV_MODE 0x2200
#endif
#ifndef GL_MODULATE
#define GL_MODULATE 0x2100
#endif
#ifndef GL_DECAL
#define GL_DECAL 0x2101
#endif
#ifndef GL_CLAMP
#define GL_CLAMP GL_CLAMP_TO_EDGE
#endif
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
#ifndef GL_COMBINE
#define GL_COMBINE 0x8570
#endif
#ifndef GL_COMBINE_RGB
#define GL_COMBINE_RGB 0x8571
#endif
#ifndef GL_ADD
#define GL_ADD 0x0104
#endif

// Alpha test constants
#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif

// Client state constants
#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY 0x8074
#endif
#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY 0x8075
#endif
#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY 0x8076
#endif
#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif

// Blend function query constants
#ifndef GL_BLEND_SRC
#define GL_BLEND_SRC 0x0BE1
#endif
#ifndef GL_BLEND_DST
#define GL_BLEND_DST 0x0BE0
#endif

// Texture formats that may need conversion
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif
#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#endif
#ifndef GL_RGB5_A1
#define GL_RGB5_A1 0x8057
#endif

// Data type for double (map to float in ES)
#ifndef GL_DOUBLE
#define GL_DOUBLE GL_FLOAT
#endif

// ============================================================================
// Extension function pointer types (for compatibility with extension loading)
// ============================================================================

typedef void (*PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (*PFNGLACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (*PFNGLMULTITEXCOORD2FARBPROC)(GLenum target, GLfloat s, GLfloat t);
typedef void (*PFNGLMULTITEXCOORD2FVARBPROC)(GLenum target, const GLfloat *v);

// Map ARB texture functions to standard ES functions
#define GL_TEXTURE0_ARB GL_TEXTURE0
#define GL_TEXTURE1_ARB GL_TEXTURE1
#define glActiveTextureARB glActiveTexture
#define glClientActiveTextureARB(texture) ((void)0)  // Stubbed - ES doesn't have this
#define glMultiTexCoord2fARB(target,s,t) ((void)0)   // Stubbed for immediate mode
#define glMultiTexCoord2fvARB(target,v) ((void)0)    // Stubbed for immediate mode

#endif // __ANDROID__
