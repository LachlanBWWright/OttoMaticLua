#pragma once

#include <stdbool.h>

#if defined(__NDS__) || defined(OTTO_DS_HOMEBREW)
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef double GLdouble;
typedef float GLclampf;
typedef void GLvoid;
typedef void (*PFNGLACTIVETEXTUREARBPROC)(GLenum);
typedef void (*PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum);

enum
{
	GL_FALSE = 0,
	GL_TRUE = 1,
	GL_NO_ERROR = 0,
	GL_BLEND = 0x0BE2,
	GL_CULL_FACE = 0x0B44,
	GL_DEPTH_TEST = 0x0B71,
	GL_TEXTURE_2D = 0x0DE1,
	GL_ALPHA_TEST = 0x0BC0,
	GL_FOG = 0x0B60,
	GL_LIGHTING = 0x0B50,
	GL_COLOR_MATERIAL = 0x0B57,
	GL_NORMALIZE = 0x0BA1,
	GL_RESCALE_NORMAL = 0x803A,
	GL_FRONT_AND_BACK = 0x0408,
	GL_AMBIENT_AND_DIFFUSE = 0x1602,
	GL_COLOR_BUFFER_BIT = 0x00004000,
	GL_DEPTH_BUFFER_BIT = 0x00000100,
	GL_SRC_ALPHA = 0x0302,
	GL_ONE_MINUS_SRC_ALPHA = 0x0303,
	GL_NOTEQUAL = 0x0205,
	GL_FOG_MODE = 0x0B65,
	GL_FOG_DENSITY = 0x0B62,
	GL_FOG_START = 0x0B63,
	GL_FOG_END = 0x0B64,
	GL_FOG_COLOR = 0x0B66,
	GL_FOG_HINT = 0x0C54,
	GL_NICEST = 0x1102,
	GL_FASTEST = 0x1101,
	GL_BACK = 0x0405,
	GL_CCW = 0x0901,
	GL_TEXTURE0_ARB = 0x84C0,
	GL_TEXTURE1_ARB = 0x84C1,
	GL_TEXTURE_ENV = 0x2300,
	GL_TEXTURE_GEN_S = 0x0C60,
	GL_TEXTURE_GEN_T = 0x0C61,
	GL_TEXTURE_GEN_MODE = 0x2500,
	GL_OBJECT_LINEAR = 0x2401,
	GL_OBJECT_PLANE = 0x2501,
	GL_SPHERE_MAP = 0x2402,
	GL_TEXTURE_COORD_ARRAY = 0x8078,
	GL_VERTEX_ARRAY = 0x8074,
	GL_COLOR_ARRAY = 0x8076,
	GL_NORMAL_ARRAY = 0x8075,
	GL_TEXTURE_WRAP_S = 0x2802,
	GL_TEXTURE_WRAP_T = 0x2803,
	GL_CLAMP_TO_EDGE = 0x812F,
	GL_REPEAT = 0x2901,
	GL_TEXTURE_MIN_FILTER = 0x2801,
	GL_TEXTURE_MAG_FILTER = 0x2800,
	GL_LINEAR = 0x2601,
	GL_RGB = 0x1907,
	GL_RGBA = 0x1908,
	GL_UNSIGNED_BYTE = 0x1401,
	GL_UNSIGNED_SHORT_1_5_5_5_REV = 0x8366,
	GL_UNSIGNED_INT_8_8_8_8 = 0x8035,
	GL_UNSIGNED_INT_8_8_8_8_REV = 0x8367,
	GL_BGRA_EXT = 0x80E1,
	GL_UNSIGNED_INT = 0x1405,
	GL_FLOAT = 0x1406,
	GL_QUADS = 0x0007,
	GL_TRIANGLES = 0x0004,
	GL_LINES = 0x0001,
	GL_LINE_LOOP = 0x0002,
	GL_LINE_STRIP = 0x0003,
	GL_MODELVIEW = 0x1700,
	GL_PROJECTION = 0x1701,
	GL_CURRENT_COLOR = 0x0B00,
	GL_LIGHT0 = 0x4000,
	GL_POSITION = 0x1203,
	GL_AMBIENT = 0x1200,
	GL_DIFFUSE = 0x1201,
	GL_LIGHT_MODEL_AMBIENT = 0x0B53,
	GL_PROJECTION_MATRIX = 0x0BA7,
	GL_DEPTH_WRITEMASK = 0x0B72,
	GL_ONE = 1,
	GL_TEXTURE = 0x1702,
	GL_TEXTURE_ENV_MODE = 0x2200,
	GL_COMBINE = 0x8570,
	GL_COMBINE_RGB = 0x8571,
	GL_COMBINE_EXT = 0x8570,
	GL_COMBINE_RGB_EXT = 0x8571,
	GL_MODULATE = 0x2100,
	GL_TRANSFORM_HINT_APPLE = 0x26A0,
	GL_FILL = 0x1B02,
	GL_LINE = 0x1B01,
	GL_MAX_TEXTURE_SIZE = 0x0D33,
	GL_RENDERER = 0x1F01,
	GL_VERSION = 0x1F02,
	GL_UNPACK_ALIGNMENT = 0x0CF5,
	GL_S = 0x2000,
	GL_T = 0x2001,
	GL_EQUAL = 0x0202,
	GL_ALIASED_LINE_WIDTH_RANGE = 0x846E,
	GL_RGB5_A1 = 0x8057
};
#endif

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
