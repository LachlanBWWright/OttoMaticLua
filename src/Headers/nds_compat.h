//
// nds_compat.h
// Nintendo DS compatibility layer - provides type mappings and platform abstractions
// for building OttoMatic as a Nintendo DS homebrew application.
//
// The DS uses libnds with videoGL for 3D rendering, which provides an OpenGL 1.x-like
// API but with fixed-point math instead of floating-point.
//

#pragma once

#ifdef NDS

#include <nds.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/trig_lut.h>
#include <nds/arm9/math.h>
#include <nds/arm9/image.h>
#include <nds/arm9/input.h>
#include <nds/arm9/console.h>
#include <nds/arm9/background.h>
#include <nds/interrupts.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/****************************/
/*    NDS SCREEN CONSTANTS  */
/****************************/

#define NDS_SCREEN_WIDTH    256
#define NDS_SCREEN_HEIGHT   192
#define NDS_VRAM_SIZE       (256 * 1024)  // 256KB total VRAM
#define NDS_MAX_POLYGONS    2048
#define NDS_MAX_VERTICES    6144

/****************************/
/*    GL TYPE DEFINITIONS   */
/****************************/

// The NDS videoGL already defines many GL types, but we need to ensure
// compatibility with the desktop OpenGL types used throughout the codebase.
// videoGL uses fixed-point math (v16, f32, etc.) internally.

// Ensure standard GL types are available
#ifndef GL_TYPES_DEFINED
#define GL_TYPES_DEFINED

typedef float           GLfloat;
typedef double          GLdouble;
typedef int             GLint;
typedef unsigned int    GLuint;
typedef unsigned int    GLenum;
typedef unsigned char   GLubyte;
typedef unsigned char   GLboolean;
typedef int             GLsizei;
typedef void            GLvoid;
typedef unsigned int    GLbitfield;
typedef short           GLshort;
typedef unsigned short  GLushort;
typedef signed char     GLbyte;
typedef float           GLclampf;
typedef double          GLclampd;

#endif // GL_TYPES_DEFINED

/****************************/
/*    GL CONSTANTS          */
/****************************/

// These constants may not all be defined by videoGL but are used by the game code.
// We define them here as NDS equivalents or no-ops.

#ifndef GL_FRONT_AND_BACK
#define GL_FRONT_AND_BACK       0x0408
#endif

#ifndef GL_AMBIENT_AND_DIFFUSE
#define GL_AMBIENT_AND_DIFFUSE  0x1602
#endif

#ifndef GL_EMISSION
#define GL_EMISSION             0x1600
#endif

#ifndef GL_AMBIENT
#define GL_AMBIENT              0x1200
#endif

#ifndef GL_DIFFUSE
#define GL_DIFFUSE              0x1201
#endif

#ifndef GL_SPECULAR
#define GL_SPECULAR             0x1202
#endif

#ifndef GL_SHININESS
#define GL_SHININESS            0x1601
#endif

#ifndef GL_POSITION
#define GL_POSITION             0x1203
#endif

#ifndef GL_LIGHT_MODEL_AMBIENT
#define GL_LIGHT_MODEL_AMBIENT  0x0B53
#endif

// Texture environment constants
#ifndef GL_TEXTURE_ENV
#define GL_TEXTURE_ENV          0x2300
#endif

#ifndef GL_TEXTURE_ENV_MODE
#define GL_TEXTURE_ENV_MODE     0x2200
#endif

#ifndef GL_MODULATE
#define GL_MODULATE             0x2100
#endif

#ifndef GL_DECAL
#define GL_DECAL                0x2101
#endif

#ifndef GL_ADD
#define GL_ADD                  0x0104
#endif

// Fog constants
#ifndef GL_FOG_MODE
#define GL_FOG_MODE             0x0B65
#endif

#ifndef GL_FOG_DENSITY
#define GL_FOG_DENSITY          0x0B62
#endif

#ifndef GL_FOG_START
#define GL_FOG_START            0x0B63
#endif

#ifndef GL_FOG_END
#define GL_FOG_END              0x0B64
#endif

#ifndef GL_FOG_COLOR
#define GL_FOG_COLOR            0x0B66
#endif

#ifndef GL_EXP
#define GL_EXP                  0x0800
#endif

#ifndef GL_EXP2
#define GL_EXP2                 0x0801
#endif

// Additional GL constants used by the game
#ifndef GL_LINEAR
#define GL_LINEAR               0x2601
#endif

#ifndef GL_NEAREST
#define GL_NEAREST              0x2600
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE        0x812F
#endif

#ifndef GL_REPEAT
#define GL_REPEAT               0x2901
#endif

#ifndef GL_TEXTURE_WRAP_S
#define GL_TEXTURE_WRAP_S       0x2802
#endif

#ifndef GL_TEXTURE_WRAP_T
#define GL_TEXTURE_WRAP_T       0x2803
#endif

#ifndef GL_TEXTURE_MIN_FILTER
#define GL_TEXTURE_MIN_FILTER   0x2801
#endif

#ifndef GL_TEXTURE_MAG_FILTER
#define GL_TEXTURE_MAG_FILTER   0x2800
#endif

#ifndef GL_RGBA
#define GL_RGBA                 0x1908
#endif

#ifndef GL_RGB
#define GL_RGB                  0x1907
#endif

#ifndef GL_UNSIGNED_BYTE
#define GL_UNSIGNED_BYTE        0x1401
#endif

#ifndef GL_UNSIGNED_SHORT
#define GL_UNSIGNED_SHORT       0x1403
#endif

#ifndef GL_FLOAT
#define GL_FLOAT                0x1406
#endif

#ifndef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT     0x00004000
#endif

#ifndef GL_DEPTH_BUFFER_BIT
#define GL_DEPTH_BUFFER_BIT     0x00000100
#endif

// Alpha test functions
#ifndef GL_NEVER
#define GL_NEVER                0x0200
#endif

#ifndef GL_LESS
#define GL_LESS                 0x0201
#endif

#ifndef GL_EQUAL
#define GL_EQUAL                0x0202
#endif

#ifndef GL_LEQUAL
#define GL_LEQUAL               0x0203
#endif

#ifndef GL_GREATER
#define GL_GREATER              0x0204
#endif

#ifndef GL_NOTEQUAL
#define GL_NOTEQUAL             0x0205
#endif

#ifndef GL_GEQUAL
#define GL_GEQUAL               0x0206
#endif

#ifndef GL_ALWAYS
#define GL_ALWAYS               0x0207
#endif

// Blend functions
#ifndef GL_SRC_ALPHA
#define GL_SRC_ALPHA            0x0302
#endif

#ifndef GL_ONE_MINUS_SRC_ALPHA
#define GL_ONE_MINUS_SRC_ALPHA  0x0303
#endif

#ifndef GL_ONE
#define GL_ONE                  0x0001
#endif

#ifndef GL_ZERO
#define GL_ZERO                 0x0000
#endif

// Primitives
#ifndef GL_POINTS
#define GL_POINTS               0x0000
#endif

#ifndef GL_LINES
#define GL_LINES                0x0001
#endif

#ifndef GL_LINE_STRIP
#define GL_LINE_STRIP           0x0003
#endif

#ifndef GL_TRIANGLES
#define GL_TRIANGLES            0x0004
#endif

#ifndef GL_TRIANGLE_STRIP
#define GL_TRIANGLE_STRIP       0x0005
#endif

#ifndef GL_TRIANGLE_FAN
#define GL_TRIANGLE_FAN         0x0006
#endif

#ifndef GL_QUADS
#define GL_QUADS                0x0007
#endif

#ifndef GL_QUAD_STRIP
#define GL_QUAD_STRIP           0x0008
#endif

// Enable/Disable capabilities
#ifndef GL_LIGHTING
#define GL_LIGHTING             0x0B50
#endif

#ifndef GL_LIGHT0
#define GL_LIGHT0               0x4000
#endif

#ifndef GL_LIGHT1
#define GL_LIGHT1               0x4001
#endif

#ifndef GL_LIGHT2
#define GL_LIGHT2               0x4002
#endif

#ifndef GL_LIGHT3
#define GL_LIGHT3               0x4003
#endif

#ifndef GL_DEPTH_TEST
#define GL_DEPTH_TEST           0x0B71
#endif

#ifndef GL_CULL_FACE
#define GL_CULL_FACE            0x0B44
#endif

#ifndef GL_NORMALIZE
#define GL_NORMALIZE            0x0BA1
#endif

#ifndef GL_TEXTURE_2D
#define GL_TEXTURE_2D           0x0DE1
#endif

#ifndef GL_BLEND
#define GL_BLEND                0x0BE2
#endif

#ifndef GL_FOG
#define GL_FOG                  0x0B60
#endif

#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST           0x0BC0
#endif

#ifndef GL_COLOR_MATERIAL
#define GL_COLOR_MATERIAL       0x0B57
#endif

#ifndef GL_FRONT
#define GL_FRONT                0x0404
#endif

#ifndef GL_BACK
#define GL_BACK                 0x0405
#endif

// Texture coordinate generation
#ifndef GL_TEXTURE_GEN_S
#define GL_TEXTURE_GEN_S        0x0C60
#endif

#ifndef GL_TEXTURE_GEN_T
#define GL_TEXTURE_GEN_T        0x0C61
#endif

#ifndef GL_TEXTURE_GEN_MODE
#define GL_TEXTURE_GEN_MODE     0x2500
#endif

#ifndef GL_SPHERE_MAP
#define GL_SPHERE_MAP           0x2402
#endif

#ifndef GL_S
#define GL_S                    0x2000
#endif

#ifndef GL_T
#define GL_T                    0x2001
#endif

// Client state
#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY         0x8074
#endif

#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY         0x8075
#endif

#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY          0x8076
#endif

#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY  0x8078
#endif

// Matrix modes
#ifndef GL_MODELVIEW
#define GL_MODELVIEW            0x1700
#endif

#ifndef GL_PROJECTION
#define GL_PROJECTION           0x1701
#endif

#ifndef GL_TEXTURE
#define GL_TEXTURE              0x1702
#endif

// String queries
#ifndef GL_VENDOR
#define GL_VENDOR               0x1F00
#endif

#ifndef GL_RENDERER
#define GL_RENDERER             0x1F01
#endif

#ifndef GL_VERSION
#define GL_VERSION              0x1F02
#endif

#ifndef GL_EXTENSIONS
#define GL_EXTENSIONS           0x1F03
#endif

// Pixel store
#ifndef GL_UNPACK_ALIGNMENT
#define GL_UNPACK_ALIGNMENT     0x0CF5
#endif

#ifndef GL_PACK_ALIGNMENT
#define GL_PACK_ALIGNMENT       0x0D05
#endif

// Error codes
#ifndef GL_NO_ERROR
#define GL_NO_ERROR             0
#endif

// Polygon mode (not supported on NDS, will be no-op)
#ifndef GL_FILL
#define GL_FILL                 0x1B02
#endif

#ifndef GL_LINE
#define GL_LINE                 0x1B01
#endif

#ifndef GL_POINT
#define GL_POINT                0x1B00
#endif

// Fog hint
#ifndef GL_FOG_HINT
#define GL_FOG_HINT             0x0C54
#endif

#ifndef GL_DONT_CARE
#define GL_DONT_CARE            0x1100
#endif

#ifndef GL_NICEST
#define GL_NICEST               0x1102
#endif

// Multi-texture
#ifndef GL_TEXTURE0
#define GL_TEXTURE0             0x84C0
#endif

#ifndef GL_TEXTURE1
#define GL_TEXTURE1             0x84C1
#endif

#ifndef GL_TEXTURE0_ARB
#define GL_TEXTURE0_ARB         GL_TEXTURE0
#endif

#ifndef GL_TEXTURE1_ARB
#define GL_TEXTURE1_ARB         GL_TEXTURE1
#endif

// GL extension function pointer types (stubs for NDS)
typedef void (*PFNGLACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (*PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum texture);

/****************************/
/*    SDL TYPE STUBS        */
/****************************/

// Minimal SDL type stubs for NDS (replaces SDL3 dependency)
typedef void* SDL_Window;
typedef void* SDL_GLContext;
typedef void* SDL_Gamepad;
typedef uint32_t SDL_GamepadButton;
typedef uint32_t SDL_Keycode;
typedef uint32_t SDL_Scancode;

// SDL log stub
#define SDL_Log(fmt, ...) iprintf(fmt "\n", ##__VA_ARGS__)

// SDL hide cursor stub
#define SDL_HideCursor() ((void)0)

// SDL GL stubs
#define SDL_GL_MakeCurrent(w, c) ((void)0)
#define SDL_GL_SwapWindow(w) NDS_SwapBuffers()
#define SDL_GL_SetSwapInterval(i) ((void)0)

// SDL timer
#define SDL_GetTicks() NDS_GetTicks()

/****************************/
/*    NDS HELPER FUNCTIONS  */
/****************************/

// Fixed-point conversion helpers
// NDS uses v16 (4.12 fixed point) for vertices and f32 (20.12 fixed point) for matrices

static inline v16 NDS_FloatToV16(float f)
{
    return floattov16(f);
}

static inline f32 NDS_FloatToF32(float f)
{
    return floattof32(f);
}

static inline float NDS_V16ToFloat(v16 v)
{
    return v16tofloat(v);
}

static inline float NDS_F32ToFloat(f32 f)
{
    return f32tofloat(f);
}

// Convert a float-based 4x4 matrix to NDS m4x4 (fixed-point)
static inline void NDS_FloatMatrixToFixed(const float* src, m4x4* dst)
{
    for (int i = 0; i < 16; i++)
    {
        dst->m[i] = floattof32(src[i]);
    }
}

// Convert RGB float (0-1) to NDS RGB15
static inline uint16_t NDS_FloatColorToRGB15(float r, float g, float b)
{
    int ri = (int)(r * 31.0f);
    int gi = (int)(g * 31.0f);
    int bi = (int)(b * 31.0f);
    if (ri > 31) ri = 31; if (ri < 0) ri = 0;
    if (gi > 31) gi = 31; if (gi < 0) gi = 0;
    if (bi > 31) bi = 31; if (bi < 0) bi = 0;
    return RGB15(ri, gi, bi);
}

// Convert RGBA float (0-1) to NDS RGB15 with alpha bit
static inline uint16_t NDS_FloatColorToRGB15A(float r, float g, float b, float a)
{
    uint16_t color = NDS_FloatColorToRGB15(r, g, b);
    if (a > 0.5f)
        color |= BIT(15);  // Set alpha bit
    return color;
}

// Forward declarations for NDS platform functions
void NDS_InitGraphics(void);
void NDS_SwapBuffers(void);
uint32_t NDS_GetTicks(void);
void NDS_WaitVBlank(void);

/****************************/
/*  POMME COMPAT STUBS      */
/****************************/

// Minimal stubs for Pomme types that the game code uses
// These should eventually be provided by a NDS-compatible Pomme build

#ifndef POMME_STUBS_DEFINED
#define POMME_STUBS_DEFINED

// FSSpec is used throughout for file references
// On NDS we'll use FAT filesystem paths

#endif // POMME_STUBS_DEFINED

#endif // NDS
