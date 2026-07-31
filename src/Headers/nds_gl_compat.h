//
// nds_gl_compat.h
// Nintendo DS GL compatibility layer
//
// Maps legacy/desktop OpenGL calls to NDS videoGL equivalents.
// The NDS videoGL provides an OpenGL 1.x-like API but uses fixed-point math.
// This header provides macro redirections similar to the Emscripten gl_compat.h.
//

#pragma once

#ifdef NDS

#include "nds_compat.h"

/****************************/
/*    STATE TRACKING        */
/****************************/

// NDS GL state that we need to track since videoGL doesn't expose all state queries
typedef struct
{
    // Current matrix mode
    GLenum matrixMode;

    // Lighting state
    int lightingEnabled;
    int lightEnabled[4];

    // Texture state
    int texture2DEnabled;
    int activeTextureUnit;    // 0 = GL_TEXTURE0, 1 = GL_TEXTURE1

    // Blend state
    int blendEnabled;
    GLenum blendSrc;
    GLenum blendDst;

    // Depth state
    int depthTestEnabled;
    int depthMaskEnabled;

    // Cull face
    int cullFaceEnabled;

    // Fog
    int fogEnabled;
    float fogStart;
    float fogEnd;
    float fogDensity;
    int fogMode;
    float fogColor[4];

    // Alpha test
    int alphaTestEnabled;
    GLenum alphaFunc;
    float alphaRef;

    // Normalize
    int normalizeEnabled;

    // Color material
    int colorMaterialEnabled;

    // Current color
    float currentColor[4];

    // Current normal
    float currentNormal[3];

    // Current texcoord
    float currentTexCoord[2];

    // Material properties
    float materialAmbient[4];
    float materialDiffuse[4];
    float materialSpecular[4];
    float materialEmission[4];

    // Texture gen
    int texGenSEnabled;
    int texGenTEnabled;

    // Current bound texture
    GLuint currentTexture;

    // Polygon count tracking (NDS has 2048 limit)
    int polyCount;

    // Vertex count tracking (NDS has 6144 limit)
    int vertexCount;

} NDS_GLState;

extern NDS_GLState gNDS_GLState;

/****************************/
/*    FUNCTION DECLARATIONS */
/****************************/

// Core state management
void NDS_glEnable(GLenum cap);
void NDS_glDisable(GLenum cap);
GLboolean NDS_glIsEnabled(GLenum cap);

// Clear
void NDS_glClear(GLbitfield mask);
void NDS_glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

// Matrix operations
void NDS_glMatrixMode(GLenum mode);
void NDS_glLoadIdentity(void);
void NDS_glLoadMatrixf(const GLfloat* m);
void NDS_glMultMatrixf(const GLfloat* m);
void NDS_glPushMatrix(void);
void NDS_glPopMatrix(void);
void NDS_glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void NDS_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void NDS_glScalef(GLfloat x, GLfloat y, GLfloat z);
void NDS_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void NDS_glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);

// Viewport
void NDS_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

// Immediate mode
void NDS_glBegin(GLenum mode);
void NDS_glEnd(void);
void NDS_glVertex2f(GLfloat x, GLfloat y);
void NDS_glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void NDS_glVertex3fv(const GLfloat* v);
void NDS_glNormal3f(GLfloat x, GLfloat y, GLfloat z);
void NDS_glColor3f(GLfloat r, GLfloat g, GLfloat b);
void NDS_glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void NDS_glColor4fv(const GLfloat* v);
void NDS_glTexCoord2f(GLfloat u, GLfloat v);
void NDS_glTexCoord2fv(const GLfloat* v);

// Texture management
void NDS_glGenTextures(GLsizei n, GLuint* textures);
void NDS_glDeleteTextures(GLsizei n, const GLuint* textures);
void NDS_glBindTexture(GLenum target, GLuint texture);
void NDS_glTexImage2D(GLenum target, GLint level, GLint internalformat,
                      GLsizei width, GLsizei height, GLint border,
                      GLenum format, GLenum type, const void* data);
void NDS_glTexParameteri(GLenum target, GLenum pname, GLint param);
void NDS_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void NDS_glTexEnvi(GLenum target, GLenum pname, GLint param);
void NDS_glTexGeni(GLenum coord, GLenum pname, GLint param);

// Lighting
void NDS_glLightfv(GLenum light, GLenum pname, const GLfloat* params);
void NDS_glLightModelfv(GLenum pname, const GLfloat* params);
void NDS_glMaterialfv(GLenum face, GLenum pname, const GLfloat* params);

// Fog
void NDS_glFogf(GLenum pname, GLfloat param);
void NDS_glFogfv(GLenum pname, const GLfloat* params);
void NDS_glFogi(GLenum pname, GLint param);

// Alpha test
void NDS_glAlphaFunc(GLenum func, GLfloat ref);

// Blend
void NDS_glBlendFunc(GLenum sfactor, GLenum dfactor);

// Depth
void NDS_glDepthMask(GLboolean flag);

// Color mask
void NDS_glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a);

// Vertex arrays (client-side)
void NDS_glEnableClientState(GLenum array);
void NDS_glDisableClientState(GLenum array);
void NDS_glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
void NDS_glNormalPointer(GLenum type, GLsizei stride, const void* pointer);
void NDS_glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
void NDS_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
void NDS_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
void NDS_glDrawArrays(GLenum mode, GLint first, GLsizei count);

// Multi-texture
void NDS_glActiveTexture(GLenum texture);
void NDS_glClientActiveTexture(GLenum texture);

// State query
void NDS_glGetFloatv(GLenum pname, GLfloat* params);
void NDS_glGetIntegerv(GLenum pname, GLint* params);
void NDS_glGetBooleanv(GLenum pname, GLboolean* params);
const GLubyte* NDS_glGetString(GLenum name);
GLenum NDS_glGetError(void);

// Misc
void NDS_glHint(GLenum target, GLenum mode);
void NDS_glPixelStorei(GLenum pname, GLint param);
void NDS_glLineWidth(GLfloat width);
void NDS_glFrontFace(GLenum mode);
void NDS_glCullFace(GLenum mode);
void NDS_glPolygonMode(GLenum face, GLenum mode);

// Buffer stubs (NDS doesn't use VBOs but we need the API)
void NDS_glGenBuffers(GLsizei n, GLuint* buffers);
void NDS_glDeleteBuffers(GLsizei n, const GLuint* buffers);
void NDS_glBindBuffer(GLenum target, GLuint buffer);
void NDS_glBufferData(GLenum target, GLsizei size, const void* data, GLenum usage);

// Shader stubs (NDS has fixed-function pipeline, these are no-ops)
GLuint NDS_glCreateShader(GLenum type);
void NDS_glShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length);
void NDS_glCompileShader(GLuint shader);
void NDS_glGetShaderiv(GLuint shader, GLenum pname, GLint* params);
void NDS_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, char* infoLog);
GLuint NDS_glCreateProgram(void);
void NDS_glAttachShader(GLuint program, GLuint shader);
void NDS_glBindAttribLocation(GLuint program, GLuint index, const char* name);
void NDS_glLinkProgram(GLuint program);
void NDS_glGetProgramiv(GLuint program, GLenum pname, GLint* params);
void NDS_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei* length, char* infoLog);
void NDS_glValidateProgram(GLuint program);
void NDS_glUseProgram(GLuint program);
GLint NDS_glGetUniformLocation(GLuint program, const char* name);
void NDS_glUniform1f(GLint location, GLfloat v0);
void NDS_glUniform1i(GLint location, GLint v0);
void NDS_glUniform3fv(GLint location, GLsizei count, const GLfloat* value);
void NDS_glUniform4fv(GLint location, GLsizei count, const GLfloat* value);
void NDS_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void NDS_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void NDS_glEnableVertexAttribArray(GLuint index);
void NDS_glDisableVertexAttribArray(GLuint index);
void NDS_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

/****************************/
/*    MACRO REDIRECTIONS    */
/****************************/

// Core state
#define glEnable                NDS_glEnable
#define glDisable               NDS_glDisable
#define glIsEnabled             NDS_glIsEnabled

// Clear
#define glClear                 NDS_glClear
#define glClearColor            NDS_glClearColor

// Matrix operations
#define glMatrixMode            NDS_glMatrixMode
#define glLoadIdentity          NDS_glLoadIdentity
#define glLoadMatrixf           NDS_glLoadMatrixf
#define glMultMatrixf           NDS_glMultMatrixf
#define glPushMatrix            NDS_glPushMatrix
#define glPopMatrix             NDS_glPopMatrix
#define glTranslatef            NDS_glTranslatef
#define glRotatef               NDS_glRotatef
#define glScalef                NDS_glScalef
#define glFrustum               NDS_glFrustum
#define glOrtho                 NDS_glOrtho

// Viewport
#define glViewport              NDS_glViewport

// Immediate mode
#define glBegin                 NDS_glBegin
#define glEnd                   NDS_glEnd
#define glVertex2f              NDS_glVertex2f
#define glVertex3f              NDS_glVertex3f
#define glVertex3fv             NDS_glVertex3fv
#define glNormal3f              NDS_glNormal3f
#define glColor3f               NDS_glColor3f
#define glColor4f               NDS_glColor4f
#define glColor4fv              NDS_glColor4fv
#define glTexCoord2f            NDS_glTexCoord2f
#define glTexCoord2fv           NDS_glTexCoord2fv

// Texture management
#define glGenTextures           NDS_glGenTextures
#define glDeleteTextures        NDS_glDeleteTextures
#define glBindTexture           NDS_glBindTexture
#define glTexImage2D            NDS_glTexImage2D
#define glTexParameteri         NDS_glTexParameteri
#define glTexParameterf         NDS_glTexParameterf
#define glTexEnvi               NDS_glTexEnvi
#define glTexGeni               NDS_glTexGeni

// Lighting
#define glLightfv               NDS_glLightfv
#define glLightModelfv          NDS_glLightModelfv
#define glMaterialfv            NDS_glMaterialfv

// Fog
#define glFogf                  NDS_glFogf
#define glFogfv                 NDS_glFogfv
#define glFogi                  NDS_glFogi

// Alpha test
#define glAlphaFunc             NDS_glAlphaFunc

// Blend
#define glBlendFunc             NDS_glBlendFunc

// Depth
#define glDepthMask             NDS_glDepthMask

// Color mask
#define glColorMask             NDS_glColorMask

// Vertex arrays
#define glEnableClientState     NDS_glEnableClientState
#define glDisableClientState    NDS_glDisableClientState
#define glVertexPointer         NDS_glVertexPointer
#define glNormalPointer         NDS_glNormalPointer
#define glColorPointer          NDS_glColorPointer
#define glTexCoordPointer       NDS_glTexCoordPointer
#define glDrawElements          NDS_glDrawElements
#define glDrawArrays            NDS_glDrawArrays

// Multi-texture
#define glActiveTexture         NDS_glActiveTexture
#define glActiveTextureARB      NDS_glActiveTexture
#define glClientActiveTexture   NDS_glClientActiveTexture
#define glClientActiveTextureARB NDS_glClientActiveTexture

// State queries
#define glGetFloatv             NDS_glGetFloatv
#define glGetIntegerv           NDS_glGetIntegerv
#define glGetBooleanv           NDS_glGetBooleanv
#define glGetString             NDS_glGetString
#define glGetError              NDS_glGetError

// Misc
#define glHint                  NDS_glHint
#define glPixelStorei           NDS_glPixelStorei
#define glLineWidth             NDS_glLineWidth
#define glFrontFace             NDS_glFrontFace
#define glCullFace              NDS_glCullFace
#define glPolygonMode           NDS_glPolygonMode

// Buffer stubs
#define glGenBuffers            NDS_glGenBuffers
#define glDeleteBuffers         NDS_glDeleteBuffers
#define glBindBuffer            NDS_glBindBuffer
#define glBufferData            NDS_glBufferData

// Shader stubs (all no-ops on NDS)
#define glCreateShader          NDS_glCreateShader
#define glShaderSource          NDS_glShaderSource
#define glCompileShader         NDS_glCompileShader
#define glGetShaderiv           NDS_glGetShaderiv
#define glGetShaderInfoLog      NDS_glGetShaderInfoLog
#define glCreateProgram         NDS_glCreateProgram
#define glAttachShader          NDS_glAttachShader
#define glBindAttribLocation    NDS_glBindAttribLocation
#define glLinkProgram           NDS_glLinkProgram
#define glGetProgramiv          NDS_glGetProgramiv
#define glGetProgramInfoLog     NDS_glGetProgramInfoLog
#define glValidateProgram       NDS_glValidateProgram
#define glUseProgram            NDS_glUseProgram
#define glGetUniformLocation    NDS_glGetUniformLocation
#define glUniform1f             NDS_glUniform1f
#define glUniform1i             NDS_glUniform1i
#define glUniform3fv            NDS_glUniform3fv
#define glUniform4fv            NDS_glUniform4fv
#define glUniformMatrix3fv      NDS_glUniformMatrix3fv
#define glUniformMatrix4fv      NDS_glUniformMatrix4fv
#define glEnableVertexAttribArray   NDS_glEnableVertexAttribArray
#define glDisableVertexAttribArray  NDS_glDisableVertexAttribArray
#define glVertexAttribPointer   NDS_glVertexAttribPointer

#endif // NDS
