#include "graphics_api.h"

#if defined(__NDS__) || defined(OTTO_DS_HOMEBREW)

#include <stddef.h>
#include <string.h>

void GraphicsApi_Initialize(void)
{
}

void GraphicsApi_Shutdown(void)
{
}

void GraphicsApi_SetViewport(int x, int y, int width, int height)
{
	(void)x;
	(void)y;
	(void)width;
	(void)height;
}

void GraphicsApi_SetClearColor(float r, float g, float b, float a)
{
	(void)r;
	(void)g;
	(void)b;
	(void)a;
}

void GraphicsApi_Clear(int mask)
{
	(void)mask;
}

void GraphicsApi_SetEnable(GraphicsApiFeature feature, bool enabled)
{
	(void)feature;
	(void)enabled;
}

void GraphicsApi_SetBlendFunc(int srcBlend, int dstBlend)
{
	(void)srcBlend;
	(void)dstBlend;
}

void GraphicsApi_SetColor(float r, float g, float b, float a)
{
	(void)r;
	(void)g;
	(void)b;
	(void)a;
}

void GraphicsApi_BeginFrame(void)
{
}

void GraphicsApi_EndFrame(void)
{
}

const char* GraphicsApi_GetBackendName(void)
{
	return "Nintendo DS homebrew";
}

void glActiveTextureARB(GLenum texture)
{
	(void)texture;
}

void glClientActiveTextureARB(GLenum texture)
{
	(void)texture;
}

void glAlphaFunc(GLenum func, GLfloat ref)
{
	(void)func;
	(void)ref;
}

void glBegin(GLenum mode)
{
	(void)mode;
}

void glBindTexture(GLenum target, GLuint texture)
{
	(void)target;
	(void)texture;
}

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	(void)sfactor;
	(void)dfactor;
}

void glClear(GLbitfield mask)
{
	(void)mask;
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	(void)red;
	(void)green;
	(void)blue;
	(void)alpha;
}

void glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	(void)red;
	(void)green;
	(void)blue;
}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	(void)red;
	(void)green;
	(void)blue;
	(void)alpha;
}

void glColor4fv(const GLfloat* v)
{
	(void)v;
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	(void)red;
	(void)green;
	(void)blue;
	(void)alpha;
}

void glColorMaterial(GLenum face, GLenum mode)
{
	(void)face;
	(void)mode;
}

void glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	(void)size;
	(void)type;
	(void)stride;
	(void)pointer;
}

void glCullFace(GLenum mode)
{
	(void)mode;
}

void glDeleteTextures(GLsizei n, const GLuint* textures)
{
	(void)n;
	(void)textures;
}

void glDepthMask(GLboolean flag)
{
	(void)flag;
}

void glDisable(GLenum capability)
{
	(void)capability;
}

void glDisableClientState(GLenum array)
{
	(void)array;
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
	(void)mode;
	(void)count;
	(void)type;
	(void)indices;
}

void glEnable(GLenum capability)
{
	(void)capability;
}

void glEnableClientState(GLenum array)
{
	(void)array;
}

void glEnd(void)
{
}

void glFogf(GLenum pname, GLfloat param)
{
	(void)pname;
	(void)param;
}

void glFogfv(GLenum pname, const GLfloat* params)
{
	(void)pname;
	(void)params;
}

void glFogi(GLenum pname, GLint param)
{
	(void)pname;
	(void)param;
}

void glFrontFace(GLenum mode)
{
	(void)mode;
}

void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	(void)left;
	(void)right;
	(void)bottom;
	(void)top;
	(void)zNear;
	(void)zFar;
}

void glGenTextures(GLsizei n, GLuint* textures)
{
	(void)n;
	(void)textures;
}

void glGetBooleanv(GLenum pname, GLboolean* params)
{
	(void)pname;
	if (params != NULL)
		memset(params, 0, sizeof(GLboolean));
}

GLenum glGetError(void)
{
	return GL_NO_ERROR;
}

void glGetFloatv(GLenum pname, GLfloat* params)
{
	(void)pname;
	if (params != NULL)
		memset(params, 0, sizeof(GLfloat));
}

void glGetIntegerv(GLenum pname, GLint* params)
{
	(void)pname;
	if (params != NULL)
		memset(params, 0, sizeof(GLint));
}

const GLubyte* glGetString(GLenum name)
{
	(void)name;
	return NULL;
}

void glHint(GLenum target, GLenum mode)
{
	(void)target;
	(void)mode;
}

GLboolean glIsEnabled(GLenum cap)
{
	(void)cap;
	return GL_FALSE;
}

void glLightModelfv(GLenum pname, const GLfloat* params)
{
	(void)pname;
	(void)params;
}

void glLightfv(GLenum light, GLenum pname, const GLfloat* params)
{
	(void)light;
	(void)pname;
	(void)params;
}

void glLineWidth(GLfloat width)
{
	(void)width;
}

void glLoadIdentity(void)
{
}

void glLoadMatrixf(const GLfloat* m)
{
	(void)m;
}

void glLockArraysEXT(GLint first, GLsizei count)
{
	(void)first;
	(void)count;
}

void glMaterialfv(GLenum face, GLenum pname, const GLfloat* params)
{
	(void)face;
	(void)pname;
	(void)params;
}

void glMatrixMode(GLenum mode)
{
	(void)mode;
}

void glMultMatrixf(const GLfloat* m)
{
	(void)m;
}

void glNormalPointer(GLenum type, GLsizei stride, const void* pointer)
{
	(void)type;
	(void)stride;
	(void)pointer;
}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	(void)left;
	(void)right;
	(void)bottom;
	(void)top;
	(void)zNear;
	(void)zFar;
}

void glPixelStorei(GLenum pname, GLint param)
{
	(void)pname;
	(void)param;
}

void glPolygonMode(GLenum face, GLenum mode)
{
	(void)face;
	(void)mode;
}

void glPopMatrix(void)
{
}

void glPushMatrix(void)
{
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	(void)angle;
	(void)x;
	(void)y;
	(void)z;
}

void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	(void)x;
	(void)y;
	(void)z;
}

void glTexCoord2f(GLfloat s, GLfloat t)
{
	(void)s;
	(void)t;
}

void glTexCoord2fv(const GLfloat* v)
{
	(void)v;
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	(void)size;
	(void)type;
	(void)stride;
	(void)pointer;
}

void glTexEnvi(GLenum target, GLenum pname, GLint param)
{
	(void)target;
	(void)pname;
	(void)param;
}

void glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params)
{
	(void)coord;
	(void)pname;
	(void)params;
}

void glTexGeni(GLenum coord, GLenum pname, GLint param)
{
	(void)coord;
	(void)pname;
	(void)param;
}

void glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
	(void)target;
	(void)level;
	(void)internalFormat;
	(void)width;
	(void)height;
	(void)border;
	(void)format;
	(void)type;
	(void)pixels;
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	(void)target;
	(void)pname;
	(void)param;
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	(void)target;
	(void)pname;
	(void)param;
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	(void)x;
	(void)y;
	(void)z;
}

void glUnlockArraysEXT(void)
{
}

void glVertex2f(GLfloat x, GLfloat y)
{
	(void)x;
	(void)y;
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	(void)x;
	(void)y;
	(void)z;
}

void glVertex3fv(const GLfloat* v)
{
	(void)v;
}

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	(void)size;
	(void)type;
	(void)stride;
	(void)pointer;
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	(void)x;
	(void)y;
	(void)width;
	(void)height;
}

#else

#include <SDL3/SDL_opengl.h>

static GLenum GraphicsApi_TranslateFeature(GraphicsApiFeature feature)
{
	switch (feature)
	{
		case GraphicsApiFeature_Blend: return GL_BLEND;
		case GraphicsApiFeature_CullFace: return GL_CULL_FACE;
		case GraphicsApiFeature_DepthTest: return GL_DEPTH_TEST;
		case GraphicsApiFeature_Texture2D: return GL_TEXTURE_2D;
		case GraphicsApiFeature_AlphaTest: return GL_ALPHA_TEST;
		case GraphicsApiFeature_Fog: return GL_FOG;
		case GraphicsApiFeature_Lighting: return GL_LIGHTING;
		case GraphicsApiFeature_ColorMaterial: return GL_COLOR_MATERIAL;
		case GraphicsApiFeature_Normalize: return GL_NORMALIZE;
		case GraphicsApiFeature_RescaleNormal: return GL_RESCALE_NORMAL;
		default: return 0;
	}
}

void GraphicsApi_Initialize(void)
{
}

void GraphicsApi_Shutdown(void)
{
}

void GraphicsApi_SetViewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

void GraphicsApi_SetClearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void GraphicsApi_Clear(int mask)
{
	glClear(mask);
}

void GraphicsApi_SetEnable(GraphicsApiFeature feature, bool enabled)
{
	GLenum capability = GraphicsApi_TranslateFeature(feature);
	if (capability == 0)
		return;

	if (enabled)
		glEnable(capability);
	else
		glDisable(capability);
}

void GraphicsApi_SetBlendFunc(int srcBlend, int dstBlend)
{
	glBlendFunc(srcBlend, dstBlend);
}

void GraphicsApi_SetColor(float r, float g, float b, float a)
{
	glColor4f(r, g, b, a);
}

void GraphicsApi_BeginFrame(void)
{
}

void GraphicsApi_EndFrame(void)
{
}

const char* GraphicsApi_GetBackendName(void)
{
	return "OpenGL";
}

#endif
