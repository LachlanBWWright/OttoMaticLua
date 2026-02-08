// OPENGL ES 3.0 BRIDGE HEADER FOR ANDROID
// Provides fixed-function pipeline emulation on top of GLES 3.0
// Replaces the old GLES 1.1 no-op stubs with working shader-based implementations

#pragma once

#ifdef __ANDROID__

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <android/log.h>

// GLdouble doesn't exist in GLES 3.0 - typedef it
typedef double GLdouble;

#define BRIDGE_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "GLESBridge", __VA_ARGS__)
#define BRIDGE_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GLESBridge", __VA_ARGS__)
#define BRIDGE_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GLESBridge", __VA_ARGS__)

// ============================================================================
// Constants
// ============================================================================

#define BRIDGE_MAX_LIGHTS 4
#define BRIDGE_MATRIX_STACK_DEPTH 32
#define BRIDGE_IMM_MAX_VERTICES 8192

// Fixed-function constants not in GLES 3.0
#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif
#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif
#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX 0x0BA6
#endif
#ifndef GL_PROJECTION_MATRIX
#define GL_PROJECTION_MATRIX 0x0BA7
#endif
#ifndef GL_CURRENT_COLOR
#define GL_CURRENT_COLOR 0x0B00
#endif
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif
#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif
#ifndef GL_LIGHT1
#define GL_LIGHT1 0x4001
#endif
#ifndef GL_LIGHT2
#define GL_LIGHT2 0x4002
#endif
#ifndef GL_LIGHT3
#define GL_LIGHT3 0x4003
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
#ifndef GL_LIGHT_MODEL_AMBIENT
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#endif
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
#ifndef GL_LINEAR
// Note: GL_LINEAR is 0x2601 for texture filters, but GL_LINEAR for fog is 0x2601 too.
// In desktop GL, GL_LINEAR (0x2601) is used for both. Redefine if missing.
#endif
#ifndef GL_NORMALIZE
#define GL_NORMALIZE 0x0BA1
#endif
#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 0x803A
#endif
#ifndef GL_TEXTURE_2D_BINDING
#define GL_TEXTURE_2D_BINDING 0x8069
#endif
#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif
#ifndef GL_COLOR_MATERIAL
#define GL_COLOR_MATERIAL 0x0B57
#endif
#ifndef GL_AMBIENT_AND_DIFFUSE
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#endif
#ifndef GL_FRONT_AND_BACK
#define GL_FRONT_AND_BACK 0x0408
#endif
#ifndef GL_TEXTURE_ENV
#define GL_TEXTURE_ENV 0x2300
#endif
#ifndef GL_TEXTURE_ENV_MODE
#define GL_TEXTURE_ENV_MODE 0x2200
#endif

// Missing topology constants for GLES 3.0
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_QUAD_STRIP
#define GL_QUAD_STRIP 0x0008
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif
#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif
#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif

// Vertex array client state constants
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

// Texture format constants
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
#ifndef GL_LUMINANCE
#define GL_LUMINANCE 0x1909
#endif
#ifndef GL_LUMINANCE_ALPHA
#define GL_LUMINANCE_ALPHA 0x190A
#endif
#ifndef GL_ALPHA
#define GL_ALPHA 0x1906
#endif

// Blend function query constants
#ifndef GL_BLEND_SRC
#define GL_BLEND_SRC 0x0BE1
#endif
#ifndef GL_BLEND_DST
#define GL_BLEND_DST 0x0BE0
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

// Texture environment mode constants
#ifndef GL_TEXTURE0_ARB
#define GL_TEXTURE0_ARB GL_TEXTURE0
#endif
#ifndef GL_TEXTURE1_ARB
#define GL_TEXTURE1_ARB GL_TEXTURE1
#endif
#ifndef GL_COMBINE
#define GL_COMBINE 0x8570
#endif
#ifndef GL_COMBINE_EXT
#define GL_COMBINE_EXT 0x8570
#endif
#ifndef GL_COMBINE_RGB
#define GL_COMBINE_RGB 0x8571
#endif
#ifndef GL_COMBINE_RGB_EXT
#define GL_COMBINE_RGB_EXT 0x8571
#endif
#ifndef GL_ADD
#define GL_ADD 0x0104
#endif
#ifndef GL_MODULATE
#define GL_MODULATE 0x2100
#endif

// Alpha test function constants
#ifndef GL_NOTEQUAL
// GL_NOTEQUAL should already be defined (0x0205) for depth test
#endif

// Fog hint
#ifndef GL_FOG_HINT
#define GL_FOG_HINT 0x0C54
#endif

// Extension function pointer types
typedef void (*PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (*PFNGLACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (*PFNGLMULTITEXCOORD2FARBPROC)(GLenum target, GLfloat s, GLfloat t);
typedef void (*PFNGLMULTITEXCOORD2FVARBPROC)(GLenum target, const GLfloat *v);

// ============================================================================
// Bridge data structures
// ============================================================================

typedef struct {
    GLfloat position[4];
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLboolean enabled;
} BridgeLightState;

typedef struct {
    GLfloat stack[BRIDGE_MATRIX_STACK_DEPTH][16];
    int top;
} BridgeMatrixStack;

// ============================================================================
// Bridge initialization/shutdown
// ============================================================================

void GLESBridge_Init(void);
void GLESBridge_Shutdown(void);

// ============================================================================
// Matrix operations
// ============================================================================

void bridge_MatrixMode(GLenum mode);
void bridge_PushMatrix(void);
void bridge_PopMatrix(void);
void bridge_LoadIdentity(void);
void bridge_LoadMatrixf(const GLfloat *m);
void bridge_MultMatrixf(const GLfloat *m);
void bridge_Translatef(GLfloat x, GLfloat y, GLfloat z);
void bridge_Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void bridge_Scalef(GLfloat x, GLfloat y, GLfloat z);
void bridge_Orthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal);
void bridge_Frustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal);

// ============================================================================
// Immediate mode emulation
// ============================================================================

void bridge_Begin(GLenum mode);
void bridge_End(void);
void bridge_Vertex2f(GLfloat x, GLfloat y);
void bridge_Vertex2i(GLint x, GLint y);
void bridge_Vertex3f(GLfloat x, GLfloat y, GLfloat z);
void bridge_Vertex3fv(const GLfloat *v);
void bridge_Vertex3d(GLdouble x, GLdouble y, GLdouble z);
void bridge_Vertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void bridge_TexCoord2f(GLfloat s, GLfloat t);
void bridge_TexCoord2d(GLdouble s, GLdouble t);
void bridge_TexCoord2i(GLint s, GLint t);
void bridge_TexCoord2fv(const GLfloat *v);
void bridge_Normal3f(GLfloat nx, GLfloat ny, GLfloat nz);
void bridge_Normal3fv(const GLfloat *v);
void bridge_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void bridge_Color3f(GLfloat r, GLfloat g, GLfloat b);
void bridge_Color4fv(const GLfloat *v);
void bridge_Color3fv(const GLfloat *v);

// ============================================================================
// State management
// ============================================================================

void bridge_Enable(GLenum cap);
void bridge_Disable(GLenum cap);
GLboolean bridge_IsEnabled(GLenum cap);

// ============================================================================
// Lighting
// ============================================================================

void bridge_Lightfv(GLenum light, GLenum pname, const GLfloat *params);
void bridge_LightModelfv(GLenum pname, const GLfloat *params);
void bridge_LightModelf(GLenum pname, GLfloat param);
void bridge_LightModeli(GLenum pname, GLint param);
void bridge_Materialfv(GLenum face, GLenum pname, const GLfloat *params);

// ============================================================================
// Fog
// ============================================================================

void bridge_Fogf(GLenum pname, GLfloat param);
void bridge_Fogfv(GLenum pname, const GLfloat *params);
void bridge_Fogi(GLenum pname, GLint param);

// ============================================================================
// Texture environment
// ============================================================================

void bridge_TexEnvi(GLenum target, GLenum pname, GLint param);

// ============================================================================
// Alpha test
// ============================================================================

void bridge_AlphaFunc(GLenum func, GLfloat ref);

// ============================================================================
// Vertex arrays (fixed-function style)
// ============================================================================

void bridge_EnableClientState(GLenum cap);
void bridge_DisableClientState(GLenum cap);
void bridge_VertexPointer(GLint size, GLenum type, GLsizei stride, const void *ptr);
void bridge_NormalPointer(GLenum type, GLsizei stride, const void *ptr);
void bridge_ColorPointer(GLint size, GLenum type, GLsizei stride, const void *ptr);
void bridge_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *ptr);
void bridge_ClientActiveTexture(GLenum texture);

// ============================================================================
// Draw calls (intercept to set up shader state)
// ============================================================================

void bridge_DrawArrays(GLenum mode, GLint first, GLsizei count);
void bridge_DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);

// ============================================================================
// State queries
// ============================================================================

void bridge_GetFloatv(GLenum pname, GLfloat *params);
void bridge_GetIntegerv(GLenum pname, GLint *params);
void bridge_GetBooleanv(GLenum pname, GLboolean *params);

// ============================================================================
// Misc stubs
// ============================================================================

void bridge_ColorMaterial(GLenum face, GLenum mode);
void bridge_PolygonMode(GLenum face, GLenum mode);
void bridge_TexGeni(GLenum coord, GLenum pname, GLint param);
void bridge_TexGenf(GLenum coord, GLenum pname, GLfloat param);
void bridge_TexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
void bridge_Hint(GLenum target, GLenum mode);

// ============================================================================
// Blend and depth mask tracking
// ============================================================================

void bridge_BlendFunc(GLenum sfactor, GLenum dfactor);
void bridge_DepthMask(GLboolean flag);

// ============================================================================
// Shader state synchronization (called before draw calls)
// ============================================================================

void bridge_SyncShaderState(void);

#endif // __ANDROID__
