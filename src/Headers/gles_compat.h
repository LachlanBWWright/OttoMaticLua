// OPENGL ES 3.0 COMPATIBILITY HEADER FOR ANDROID
// Redirects fixed-function OpenGL calls to the GLES 3.0 Bridge
// which provides shader-based emulation of the fixed-function pipeline

#pragma once

#ifdef __ANDROID__

#include "gles_bridge.h"

// ============================================================================
// Function redirections: Desktop OpenGL -> GLES 3.0 Bridge
// ============================================================================

// Matrix operations
#define glMatrixMode(mode)               bridge_MatrixMode(mode)
#define glPushMatrix()                   bridge_PushMatrix()
#define glPopMatrix()                    bridge_PopMatrix()
#define glLoadIdentity()                 bridge_LoadIdentity()
#define glLoadMatrixf(m)                 bridge_LoadMatrixf(m)
#define glMultMatrixf(m)                 bridge_MultMatrixf(m)
#define glTranslatef(x,y,z)             bridge_Translatef(x,y,z)
#define glRotatef(a,x,y,z)             bridge_Rotatef(a,x,y,z)
#define glScalef(x,y,z)                bridge_Scalef(x,y,z)
#define glOrtho(l,r,b,t,n,f)           bridge_Orthof((float)(l),(float)(r),(float)(b),(float)(t),(float)(n),(float)(f))
#define glFrustum(l,r,b,t,n,f)         bridge_Frustumf((float)(l),(float)(r),(float)(b),(float)(t),(float)(n),(float)(f))

// Immediate mode
#define glBegin(mode)                    bridge_Begin(mode)
#define glEnd()                          bridge_End()
#define glVertex2f(x,y)                 bridge_Vertex2f(x,y)
#define glVertex2i(x,y)                 bridge_Vertex2i(x,y)
#define glVertex3f(x,y,z)              bridge_Vertex3f(x,y,z)
#define glVertex3fv(v)                  bridge_Vertex3fv(v)
#define glVertex3d(x,y,z)              bridge_Vertex3d(x,y,z)
#define glVertex4f(x,y,z,w)            bridge_Vertex4f(x,y,z,w)
#define glTexCoord2f(s,t)              bridge_TexCoord2f(s,t)
#define glTexCoord2d(s,t)              bridge_TexCoord2d(s,t)
#define glTexCoord2i(s,t)              bridge_TexCoord2i(s,t)
#define glTexCoord2fv(v)               bridge_TexCoord2fv(v)
#define glNormal3f(x,y,z)             bridge_Normal3f(x,y,z)
#define glNormal3fv(v)                 bridge_Normal3fv(v)

// Color
#define glColor4f(r,g,b,a)            bridge_Color4f(r,g,b,a)
#define glColor3f(r,g,b)              bridge_Color3f(r,g,b)
#define glColor4fv(v)                  bridge_Color4fv(v)
#define glColor3fv(v)                  bridge_Color3fv(v)

// Enable/Disable (intercept all to handle emulated caps)
#define glEnable(cap)                    bridge_Enable(cap)
#define glDisable(cap)                   bridge_Disable(cap)
#define glIsEnabled(cap)                 bridge_IsEnabled(cap)

// Lighting
#define glLightfv(l,p,v)               bridge_Lightfv(l,p,v)
#define glLightModelfv(p,v)            bridge_LightModelfv(p,v)
#define glLightModelf(p,v)             bridge_LightModelf(p,v)
#define glLightModeli(p,v)             bridge_LightModeli(p,v)
#define glMaterialfv(f,p,v)            bridge_Materialfv(f,p,v)
#define glColorMaterial(f,m)           bridge_ColorMaterial(f,m)

// Fog
#define glFogf(p,v)                    bridge_Fogf(p,v)
#define glFogfv(p,v)                   bridge_Fogfv(p,v)
#define glFogi(p,v)                    bridge_Fogi(p,v)
#define glFogiv(p,v)                   ((void)0)

// Texture environment
#define glTexEnvi(t,p,v)               bridge_TexEnvi(t,p,v)

// Alpha test
#define glAlphaFunc(f,r)               bridge_AlphaFunc(f,r)

// Vertex arrays (fixed-function style)
#define glEnableClientState(cap)         bridge_EnableClientState(cap)
#define glDisableClientState(cap)        bridge_DisableClientState(cap)
#define glVertexPointer(s,t,st,p)      bridge_VertexPointer(s,t,st,p)
#define glNormalPointer(t,st,p)        bridge_NormalPointer(t,st,p)
#define glColorPointer(s,t,st,p)       bridge_ColorPointer(s,t,st,p)
#define glTexCoordPointer(s,t,st,p)    bridge_TexCoordPointer(s,t,st,p)
#define glClientActiveTextureARB(t)     bridge_ClientActiveTexture(t)

// Draw calls (intercept to setup shader state)
#define glDrawArrays(m,f,c)            bridge_DrawArrays(m,f,c)
#define glDrawElements(m,c,t,i)        bridge_DrawElements(m,c,t,i)

// State queries
#define glGetFloatv(p,v)               bridge_GetFloatv(p,v)
#define glGetIntegerv(p,v)             bridge_GetIntegerv(p,v)
#define glGetBooleanv(p,v)             bridge_GetBooleanv(p,v)

// Blend function tracking
#define glBlendFunc(s,d)               bridge_BlendFunc(s,d)

// Depth mask tracking
#define glDepthMask(f)                 bridge_DepthMask(f)

// Misc stubs
#define glPolygonMode(f,m)             bridge_PolygonMode(f,m)
#define glTexGeni(c,p,v)               bridge_TexGeni(c,p,v)
#define glTexGenf(c,p,v)               bridge_TexGenf(c,p,v)
#define glTexGenfv(c,p,v)              bridge_TexGenfv(c,p,v)
#define glHint(t,m)                    bridge_Hint(t,m)

// Depth functions - these exist natively in GLES 3.0 but with float params
#define glClearDepth(d)                glClearDepthf((float)(d))
#define glDepthRange(n,f)              glDepthRangef((float)(n),(float)(f))

// ============================================================================
// Constants not in OpenGL ES 3.0
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

// GL_LUMINANCE and GL_LUMINANCE_ALPHA are not core in GLES 3.0 but
// we define them for compilation; texture conversion handles the actual format
#ifndef GL_LUMINANCE
#define GL_LUMINANCE 0x1909
#endif
#ifndef GL_LUMINANCE_ALPHA
#define GL_LUMINANCE_ALPHA 0x190A
#endif
#ifndef GL_ALPHA
#define GL_ALPHA 0x1906
#endif

// Blend function query constants (for state stack)
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

// Texture environment constants
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

// Extension function pointer types for ARB multitexture compatibility
// These types are used in ogl_functions.h to define function pointers that
// map to the standard GLES glActiveTexture/glClientActiveTexture functions
typedef void (*PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (*PFNGLACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (*PFNGLMULTITEXCOORD2FARBPROC)(GLenum target, GLfloat s, GLfloat t);
typedef void (*PFNGLMULTITEXCOORD2FVARBPROC)(GLenum target, const GLfloat *v);

#endif // __ANDROID__
