#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	GraphicsApiFeature_Blend = 0,
	GraphicsApiFeature_CullFace,
	GraphicsApiFeature_DepthTest,
	GraphicsApiFeature_Texture2D,
	GraphicsApiFeature_AlphaTest,
	GraphicsApiFeature_Fog,
	GraphicsApiFeature_Lighting,
	GraphicsApiFeature_ColorMaterial,
	GraphicsApiFeature_Normalize,
	GraphicsApiFeature_RescaleNormal,
	GraphicsApiFeature_Count
} GraphicsApiFeature;

void GraphicsApi_Initialize(void);
void GraphicsApi_Shutdown(void);
void GraphicsApi_SetViewport(int x, int y, int width, int height);
void GraphicsApi_SetClearColor(float r, float g, float b, float a);
void GraphicsApi_Clear(int mask);
void GraphicsApi_SetEnable(GraphicsApiFeature feature, bool enabled);
void GraphicsApi_SetBlendFunc(int srcBlend, int dstBlend);
void GraphicsApi_SetColor(float r, float g, float b, float a);
void GraphicsApi_BeginFrame(void);
void GraphicsApi_EndFrame(void);
const char* GraphicsApi_GetBackendName(void);

#ifdef __cplusplus
}
#endif
