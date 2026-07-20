#include "graphics_api.h"

#if defined(__NDS__) || defined(OTTO_DS_HOMEBREW)

#include <stddef.h>

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
