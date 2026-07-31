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
typedef uint32_t SDL_JoystickID;
typedef uint32_t SDL_DisplayID;
typedef int16_t Sint16;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef uint8_t Uint8;
typedef int32_t Sint32;

// SDL log category stubs
#define SDL_LOG_CATEGORY_APPLICATION 0

// SDL log stub
#define SDL_Log(fmt, ...) iprintf(fmt "\n", ##__VA_ARGS__)
#define SDL_LogError(cat, fmt, ...) iprintf("ERR: " fmt "\n", ##__VA_ARGS__)

// SDL utility function stubs (map to standard C equivalents)
#define SDL_memset memset
#define SDL_memcpy memcpy
#define SDL_snprintf snprintf
#define SDL_vsnprintf vsnprintf
#define SDL_malloc malloc
#define SDL_calloc calloc
#define SDL_realloc realloc
#define SDL_free free
#define SDL_strcmp strcmp
#define SDL_strncmp strncmp
#define SDL_strlen strlen
#define SDL_strlcpy(dst, src, sz) strncpy(dst, src, sz)
#define SDL_abs abs
#define SDL_sscanf sscanf
#define SDL_strchr strchr
#define SDL_strncmp strncmp
#define SDL_qsort qsort
#define SDL_memcmp memcmp

// SDL locale stub
typedef struct { const char* language; const char* country; } SDL_Locale;
static inline SDL_Locale** SDL_GetPreferredLocales(int* count) { *count = 0; return NULL; }

// SDL scancode stubs - provide constants so code referencing them compiles
// On NDS, GetKeyState/GetNewKeyState always return false, so these are never matched
#define SDL_SCANCODE_COUNT       512
#define SDL_SCANCODE_RETURN      40
#define SDL_SCANCODE_ESCAPE      41
#define SDL_SCANCODE_LALT        226
#define SDL_SCANCODE_RALT        230
#define SDL_SCANCODE_LCTRL       224
#define SDL_SCANCODE_RCTRL       228
#define SDL_SCANCODE_LGUI        227
#define SDL_SCANCODE_RGUI        231
#define SDL_SCANCODE_LSHIFT      225
#define SDL_SCANCODE_RSHIFT      229
#define SDL_SCANCODE_KP_PLUS     87
#define SDL_SCANCODE_GRAVE       53
#define SDL_SCANCODE_F8          65
#define SDL_SCANCODE_F9          66
#define SDL_SCANCODE_F10         67
#define SDL_SCANCODE_M           16
#define SDL_SCANCODE_F           9
#define SDL_SCANCODE_W           26
#define SDL_SCANCODE_L           15
#define SDL_SCANCODE_B           5
#define SDL_SCANCODE_R           21
#define SDL_SCANCODE_I           12
#define SDL_SCANCODE_C           6
#define SDL_SCANCODE_Q           20
#define SDL_SCANCODE_MINUS       45
#define SDL_SCANCODE_EQUALS      46
#define SDL_SCANCODE_COMMA       54
#define SDL_SCANCODE_PERIOD      55
#define SDL_SCANCODE_1           30
#define SDL_SCANCODE_2           31
#define SDL_SCANCODE_3           32
#define SDL_SCANCODE_4           33
#define SDL_SCANCODE_5           34
#define SDL_SCANCODE_6           35
#define SDL_SCANCODE_7           36
#define SDL_SCANCODE_8           37
#define SDL_SCANCODE_9           38
#define SDL_SCANCODE_0           39

// SDL mouse button stubs
#define SDL_BUTTON_LEFT    1
#define SDL_BUTTON_MIDDLE  2
#define SDL_BUTTON_RIGHT   3

// SDL gamepad axis stubs
#define SDL_GAMEPAD_AXIS_LEFTX          0
#define SDL_GAMEPAD_AXIS_LEFTY          1
#define SDL_GAMEPAD_AXIS_RIGHTX         2
#define SDL_GAMEPAD_AXIS_RIGHTY         3
#define SDL_GAMEPAD_AXIS_LEFT_TRIGGER   4
#define SDL_GAMEPAD_AXIS_RIGHT_TRIGGER  5

// SDL gamepad button constants
#define SDL_GAMEPAD_BUTTON_INVALID       (-1)
#define SDL_GAMEPAD_BUTTON_SOUTH         0
#define SDL_GAMEPAD_BUTTON_EAST          1
#define SDL_GAMEPAD_BUTTON_WEST          2
#define SDL_GAMEPAD_BUTTON_NORTH         3
#define SDL_GAMEPAD_BUTTON_BACK          4
#define SDL_GAMEPAD_BUTTON_GUIDE         5
#define SDL_GAMEPAD_BUTTON_START         6
#define SDL_GAMEPAD_BUTTON_LEFT_STICK    7
#define SDL_GAMEPAD_BUTTON_RIGHT_STICK   8
#define SDL_GAMEPAD_BUTTON_LEFT_SHOULDER  9
#define SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER 10
#define SDL_GAMEPAD_BUTTON_DPAD_UP       11
#define SDL_GAMEPAD_BUTTON_DPAD_DOWN     12
#define SDL_GAMEPAD_BUTTON_DPAD_LEFT     13
#define SDL_GAMEPAD_BUTTON_DPAD_RIGHT    14
#define SDL_GAMEPAD_BUTTON_COUNT         15
#define SDL_GAMEPAD_AXIS_COUNT           6

// SDL key stubs
#define SDLK_Q  'q'
#define SDL_GetScancodeFromKey(k, m) (0)

// SDL hide cursor stub
#define SDL_HideCursor() ((void)0)
#define SDL_ShowCursor() ((void)0)
#define SDL_CursorVisible() (false)

// SDL cursor stubs
typedef void* SDL_Cursor;
#define SDL_SYSTEM_CURSOR_DEFAULT  0
#define SDL_SYSTEM_CURSOR_POINTER  1
#define SDL_CreateSystemCursor(id) (NULL)
#define SDL_SetCursor(c) ((void)0)
#define SDL_GetCursor() (NULL)
#define SDL_DestroyCursor(c) ((void)0)

// SDL mouse stubs
#define SDL_GetWindowSize(w, pw, ph) do { *(pw) = NDS_SCREEN_WIDTH; *(ph) = NDS_SCREEN_HEIGHT; } while(0)

// SDL event/pump stubs
#define SDL_PumpEvents() ((void)0)
#define SDL_FlushEvents(a, b) ((void)0)
#define SDL_WINDOWPOS_CENTERED_DISPLAY(d) (0)

// SDL window stubs
#define SDL_SetWindowFullscreen(w, f) ((void)0)
#define SDL_SetWindowSize(w, x, y) ((void)0)
#define SDL_SetWindowPosition(w, x, y) ((void)0)
#define SDL_SyncWindow(w) ((void)0)
#define SDL_GetWindowSizeInPixels(w, pw, ph) do { *(pw) = NDS_SCREEN_WIDTH; *(ph) = NDS_SCREEN_HEIGHT; } while(0)
#define SDL_GetDisplayForWindow(w) (0)
#define SDL_ShowWindow(w) ((void)0)
#define SDL_HideWindow(w) ((void)0)
#define SDL_SetWindowMouseGrab(w, g) ((void)0)
#define SDL_SetWindowRelativeMouseMode(w, m) ((void)0)
#define SDL_GetWindowRelativeMouseMode(w) (false)
#define SDL_GetMouseState(x, y) (0)
#define SDL_ShowSimpleMessageBox(f, t, m, w) ((void)0)

// SDL gamepad stubs
#define SDL_GetGamepadAxis(g, a) (0)
#define SDL_GetGamepadButton(g, b) (0)
#define SDL_GetGamepadID(g) (0)
#define SDL_RumbleGamepad(g, lo, hi, ms) ((void)0)
#define SDL_IsGamepad(id) (false)
#define SDL_OpenGamepad(id) (NULL)
#define SDL_CloseGamepad(g) ((void)0)
#define SDL_GetJoysticks(n) (*(n) = 0, (SDL_JoystickID*)NULL)
#define SDL_GetJoystickNameForID(id) ("NDS")
#define SDL_GetGamepadName(g) ("NDS")
#define SDL_GetGamepadStringForButton(b) ("?")
#define SDL_GetGamepadStringForAxis(a) ("?")
#define SDL_GetScancodeName(s) ("?")

// SDL text input stubs
#define SDL_StartTextInput(w) ((void)0)
#define SDL_StopTextInput(w) ((void)0)

// SDL window flags stubs
#define SDL_WINDOW_INPUT_FOCUS  0x0200
#define SDL_GetWindowFlags(w) (0)

// SDL ticks
#define SDL_GetTicksNS() (0ULL)

// SDL Rect type stub
typedef struct { int x, y, w, h; } SDL_Rect;
#define SDL_GetDisplayUsableBounds(d, r) ((void)0)
#define SDL_GetDisplays(n) (*(n) = 1, (SDL_DisplayID*)NULL)

// SDL keyboard stubs
#define SDL_GetKeyboardState(n) (NULL)

// SDL performance counter stubs
#define SDL_GetPerformanceCounter() (0ULL)
#define SDL_GetPerformanceFrequency() (1ULL)
#define SDL_Delay(ms) swiDelay((ms) * 8000)

// SDL fallthrough attribute
#define SDL_FALLTHROUGH /* fallthrough */

// SDL message box
#define SDL_MESSAGEBOX_ERROR    0
#define SDL_MESSAGEBOX_WARNING  1

// SDL GL stubs
#define SDL_GL_MakeCurrent(w, c) ((void)0)
#define SDL_GL_SwapWindow(w) NDS_SwapBuffers()
#define SDL_GL_SetSwapInterval(i) ((void)0)
#define SDL_GL_CreateContext(w) (NULL)
#define SDL_GL_DestroyContext(c) ((void)0)

// SDL error/info stubs
#define SDL_GetError() ("NDS: no SDL")
#define SDL_GetRevision() ("NDS")
#define SDL_GetCurrentVideoDriver() ("NDS videoGL")

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
