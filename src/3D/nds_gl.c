/****************************/
/*   NDS GL COMPATIBILITY   */
/*   Implementation         */
/****************************/

//
// nds_gl.c
// Nintendo DS OpenGL compatibility layer implementation.
// Maps desktop/mobile OpenGL calls to NDS videoGL (libnds) equivalents.
//
// The NDS 3D hardware provides a fixed-function pipeline similar to OpenGL 1.x,
// accessible through libnds's videoGL API which already uses OpenGL-like naming.
// Key differences:
//   - Fixed-point math (v16 = 4.12, f32 = 20.12) instead of floating-point
//   - 256KB VRAM for textures
//   - 2048 polygon limit per frame
//   - 6144 vertex limit per frame
//   - No programmable shaders
//   - Limited texture formats (A3I5, A5I3, 4-color, 16-color, 256-color, direct color, compressed)
//   - Single texture unit (no multi-texturing in hardware)
//

#ifdef NDS

#include "game.h"

/****************************/
/*    VARIABLES             */
/****************************/

NDS_GLState gNDS_GLState;

static uint32_t gNDS_TickCounter = 0;
static GLuint gNDS_NextTextureID = 1;
static GLuint gNDS_NextBufferID = 1;

// Texture tracking
#define NDS_MAX_TEXTURES 256
static struct {
    int used;
    int ndsTextureID;
    int width;
    int height;
    GL_TEXTURE_TYPE_ENUM type;
    int param;
} gNDS_Textures[NDS_MAX_TEXTURES];

// Client-side vertex array state
static struct {
    int vertexArrayEnabled;
    int normalArrayEnabled;
    int colorArrayEnabled;
    int texCoordArrayEnabled;

    const GLfloat* vertexPointer;
    GLint vertexSize;
    GLsizei vertexStride;

    const GLfloat* normalPointer;
    GLsizei normalStride;

    const GLfloat* colorPointer;
    GLint colorSize;
    GLsizei colorStride;

    const GLfloat* texCoordPointer;
    GLint texCoordSize;
    GLsizei texCoordStride;

    int activeClientTexture;
} gNDS_VertexArrayState;

// Immediate mode state
static GLenum gNDS_ImmediateMode = 0;

/****************************/
/*    PLATFORM FUNCTIONS    */
/****************************/

void NDS_InitGraphics(void)
{
    // Initialize NDS video hardware
    powerOn(POWER_ALL);

    // Set video mode - Mode 0 with 3D on main screen
    videoSetMode(MODE_0_3D);

    // Initialize the 3D engine
    glInit();

    // Setup VRAM banks
    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankB(VRAM_B_TEXTURE);
    vramSetBankC(VRAM_C_LCD);
    vramSetBankD(VRAM_D_LCD);

    // Enable textures
    glEnable(GL_TEXTURE_2D);

    // Enable antialiasing
    glEnable(GL_ANTIALIAS);

    // Setup the rear plane (clear color and depth)
    glClearColor(0, 0, 0, 31);
    glClearPolyID(63);
    glClearDepth(0x7FFF);

    // Set the viewport
    glViewport(0, 0, NDS_SCREEN_WIDTH - 1, NDS_SCREEN_HEIGHT - 1);

    // Initialize state tracking
    memset(&gNDS_GLState, 0, sizeof(gNDS_GLState));
    gNDS_GLState.matrixMode = GL_MODELVIEW;
    gNDS_GLState.depthTestEnabled = 1;
    gNDS_GLState.depthMaskEnabled = 1;
    gNDS_GLState.blendSrc = GL_SRC_ALPHA;
    gNDS_GLState.blendDst = GL_ONE_MINUS_SRC_ALPHA;
    gNDS_GLState.currentColor[0] = 1.0f;
    gNDS_GLState.currentColor[1] = 1.0f;
    gNDS_GLState.currentColor[2] = 1.0f;
    gNDS_GLState.currentColor[3] = 1.0f;
    gNDS_GLState.currentNormal[0] = 0.0f;
    gNDS_GLState.currentNormal[1] = 0.0f;
    gNDS_GLState.currentNormal[2] = 1.0f;
    gNDS_GLState.alphaFunc = GL_ALWAYS;
    gNDS_GLState.alphaRef = 0.0f;
    gNDS_GLState.fogMode = GL_LINEAR;

    memset(&gNDS_VertexArrayState, 0, sizeof(gNDS_VertexArrayState));
    memset(gNDS_Textures, 0, sizeof(gNDS_Textures));
}

void NDS_SwapBuffers(void)
{
    glFlush(0);
    gNDS_GLState.polyCount = 0;
    gNDS_GLState.vertexCount = 0;
}

uint32_t NDS_GetTicks(void)
{
    return gNDS_TickCounter++;
}

void NDS_WaitVBlank(void)
{
    swiWaitForVBlank();
}

/****************************/
/*    CORE STATE            */
/****************************/

void NDS_glEnable(GLenum cap)
{
    switch (cap)
    {
        case GL_LIGHTING:
            gNDS_GLState.lightingEnabled = 1;
            break;
        case GL_LIGHT0:
        case GL_LIGHT1:
        case GL_LIGHT2:
        case GL_LIGHT3:
            gNDS_GLState.lightEnabled[cap - GL_LIGHT0] = 1;
            break;
        case GL_TEXTURE_2D:
            gNDS_GLState.texture2DEnabled = 1;
            break;
        case GL_BLEND:
            gNDS_GLState.blendEnabled = 1;
            glPolyFmt(POLY_ALPHA(15) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);
            break;
        case GL_DEPTH_TEST:
            gNDS_GLState.depthTestEnabled = 1;
            break;
        case GL_CULL_FACE:
            gNDS_GLState.cullFaceEnabled = 1;
            break;
        case GL_NORMALIZE:
            gNDS_GLState.normalizeEnabled = 1;
            break;
        case GL_FOG:
            gNDS_GLState.fogEnabled = 1;
            break;
        case GL_ALPHA_TEST:
            gNDS_GLState.alphaTestEnabled = 1;
            break;
        case GL_COLOR_MATERIAL:
            gNDS_GLState.colorMaterialEnabled = 1;
            break;
        case GL_TEXTURE_GEN_S:
            gNDS_GLState.texGenSEnabled = 1;
            break;
        case GL_TEXTURE_GEN_T:
            gNDS_GLState.texGenTEnabled = 1;
            break;
        default:
            break;
    }
}

void NDS_glDisable(GLenum cap)
{
    switch (cap)
    {
        case GL_LIGHTING:
            gNDS_GLState.lightingEnabled = 0;
            break;
        case GL_LIGHT0:
        case GL_LIGHT1:
        case GL_LIGHT2:
        case GL_LIGHT3:
            gNDS_GLState.lightEnabled[cap - GL_LIGHT0] = 0;
            break;
        case GL_TEXTURE_2D:
            gNDS_GLState.texture2DEnabled = 0;
            break;
        case GL_BLEND:
            gNDS_GLState.blendEnabled = 0;
            glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
            break;
        case GL_DEPTH_TEST:
            gNDS_GLState.depthTestEnabled = 0;
            break;
        case GL_CULL_FACE:
            gNDS_GLState.cullFaceEnabled = 0;
            break;
        case GL_NORMALIZE:
            gNDS_GLState.normalizeEnabled = 0;
            break;
        case GL_FOG:
            gNDS_GLState.fogEnabled = 0;
            break;
        case GL_ALPHA_TEST:
            gNDS_GLState.alphaTestEnabled = 0;
            break;
        case GL_COLOR_MATERIAL:
            gNDS_GLState.colorMaterialEnabled = 0;
            break;
        case GL_TEXTURE_GEN_S:
            gNDS_GLState.texGenSEnabled = 0;
            break;
        case GL_TEXTURE_GEN_T:
            gNDS_GLState.texGenTEnabled = 0;
            break;
        default:
            break;
    }
}

GLboolean NDS_glIsEnabled(GLenum cap)
{
    switch (cap)
    {
        case GL_LIGHTING:       return gNDS_GLState.lightingEnabled;
        case GL_LIGHT0:         return gNDS_GLState.lightEnabled[0];
        case GL_LIGHT1:         return gNDS_GLState.lightEnabled[1];
        case GL_LIGHT2:         return gNDS_GLState.lightEnabled[2];
        case GL_LIGHT3:         return gNDS_GLState.lightEnabled[3];
        case GL_TEXTURE_2D:     return gNDS_GLState.texture2DEnabled;
        case GL_BLEND:          return gNDS_GLState.blendEnabled;
        case GL_DEPTH_TEST:     return gNDS_GLState.depthTestEnabled;
        case GL_CULL_FACE:      return gNDS_GLState.cullFaceEnabled;
        case GL_NORMALIZE:      return gNDS_GLState.normalizeEnabled;
        case GL_FOG:            return gNDS_GLState.fogEnabled;
        case GL_ALPHA_TEST:     return gNDS_GLState.alphaTestEnabled;
        case GL_COLOR_MATERIAL: return gNDS_GLState.colorMaterialEnabled;
        default:                return 0;
    }
}

/****************************/
/*    CLEAR OPERATIONS      */
/****************************/

void NDS_glClear(GLbitfield mask)
{
    // NDS clears happen automatically via the clear color/depth set in hardware
    // glClearColor and glClearDepth are set during init
    (void)mask;
}

void NDS_glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    int ri = (int)(r * 31.0f);
    int gi = (int)(g * 31.0f);
    int bi = (int)(b * 31.0f);
    int ai = (int)(a * 31.0f);
    if (ri > 31) ri = 31; if (ri < 0) ri = 0;
    if (gi > 31) gi = 31; if (gi < 0) gi = 0;
    if (bi > 31) bi = 31; if (bi < 0) bi = 0;
    if (ai > 31) ai = 31; if (ai < 0) ai = 0;
    glClearColor(ri, gi, bi, ai);
}

/****************************/
/*    MATRIX OPERATIONS     */
/****************************/

void NDS_glMatrixMode(GLenum mode)
{
    gNDS_GLState.matrixMode = mode;

    switch (mode)
    {
        case GL_PROJECTION:
            glMatrixMode(GL_PROJECTION);
            break;
        case GL_MODELVIEW:
            glMatrixMode(GL_MODELVIEW);
            break;
        case GL_TEXTURE:
            glMatrixMode(GL_TEXTURE);
            break;
    }
}

void NDS_glLoadIdentity(void)
{
    glLoadIdentity();
}

void NDS_glLoadMatrixf(const GLfloat* m)
{
    // Convert float matrix to NDS fixed-point m4x4
    m4x4 ndsMatrix;
    for (int i = 0; i < 16; i++)
    {
        ndsMatrix.m[i] = floattof32(m[i]);
    }
    glLoadMatrix4x4(&ndsMatrix);
}

void NDS_glMultMatrixf(const GLfloat* m)
{
    m4x4 ndsMatrix;
    for (int i = 0; i < 16; i++)
    {
        ndsMatrix.m[i] = floattof32(m[i]);
    }
    glMultMatrix4x4(&ndsMatrix);
}

void NDS_glPushMatrix(void)
{
    glPushMatrix();
}

void NDS_glPopMatrix(void)
{
    glPopMatrix(1);
}

void NDS_glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    glTranslatef32(floattof32(x), floattof32(y), floattof32(z));
}

void NDS_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    // NDS videoGL has glRotateXi, glRotateYi, glRotateZi for axis-aligned rotations
    // For arbitrary axis, we need to build a rotation matrix
    int angleDeg = (int)(angle * (32768.0f / 360.0f)); // Convert to NDS angle format (0-32767)

    if (x == 1.0f && y == 0.0f && z == 0.0f)
    {
        glRotateXi(angleDeg);
    }
    else if (x == 0.0f && y == 1.0f && z == 0.0f)
    {
        glRotateYi(angleDeg);
    }
    else if (x == 0.0f && y == 0.0f && z == 1.0f)
    {
        glRotateZi(angleDeg);
    }
    else
    {
        // General rotation - build rotation matrix manually
        float rad = angle * (3.14159265f / 180.0f);
        float c = cosf(rad);
        float s = sinf(rad);
        float t = 1.0f - c;
        float len = sqrtf(x*x + y*y + z*z);
        if (len > 0.0f)
        {
            x /= len; y /= len; z /= len;
        }

        float m[16];
        m[0] = t*x*x + c;      m[4] = t*x*y - s*z;    m[8]  = t*x*z + s*y;    m[12] = 0;
        m[1] = t*x*y + s*z;    m[5] = t*y*y + c;       m[9]  = t*y*z - s*x;    m[13] = 0;
        m[2] = t*x*z - s*y;    m[6] = t*y*z + s*x;     m[10] = t*z*z + c;      m[14] = 0;
        m[3] = 0;               m[7] = 0;                m[11] = 0;              m[15] = 1;

        NDS_glMultMatrixf(m);
    }
}

void NDS_glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    glScalef32(floattof32(x), floattof32(y), floattof32(z));
}

void NDS_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble znear, GLdouble zfar)
{
    // Build a frustum projection matrix manually
    float m[16];
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(zfar - znear);
    float n2 = (float)(2.0 * znear);

    memset(m, 0, sizeof(m));
    m[0]  = n2 / rl;
    m[5]  = n2 / tb;
    m[8]  = (float)(right + left) / rl;
    m[9]  = (float)(top + bottom) / tb;
    m[10] = -(float)(zfar + znear) / fn;
    m[11] = -1.0f;
    m[14] = -(float)(2.0 * zfar * znear) / fn;

    NDS_glLoadMatrixf(m);
}

void NDS_glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble znear, GLdouble zfar)
{
    glOrthof32(
        floattof32((float)left),
        floattof32((float)right),
        floattof32((float)bottom),
        floattof32((float)top),
        floattof32((float)znear),
        floattof32((float)zfar)
    );
}

/****************************/
/*    VIEWPORT              */
/****************************/

void NDS_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    // NDS viewport is always fixed to screen size, but we track it
    glViewport(x, y, width - 1, height - 1);
}

/****************************/
/*    IMMEDIATE MODE        */
/****************************/

void NDS_glBegin(GLenum mode)
{
    gNDS_ImmediateMode = mode;

    // Map GL modes to NDS equivalents
    switch (mode)
    {
        case GL_TRIANGLES:
            glBegin(GL_TRIANGLES);
            break;
        case GL_TRIANGLE_STRIP:
            glBegin(GL_TRIANGLE_STRIP);
            break;
        case GL_TRIANGLE_FAN:
            glBegin(GL_TRIANGLE_STRIP); // NDS doesn't have triangle fan, approximate
            break;
        case GL_QUADS:
            glBegin(GL_QUADS);
            break;
        case GL_QUAD_STRIP:
            glBegin(GL_QUAD_STRIP);
            break;
        default:
            glBegin(GL_TRIANGLES); // Fallback
            break;
    }
}

void NDS_glEnd(void)
{
    glEnd();
    gNDS_ImmediateMode = 0;
}

void NDS_glVertex2f(GLfloat x, GLfloat y)
{
    glVertex3v16(floattov16(x), floattov16(y), 0);
    gNDS_GLState.vertexCount++;
}

void NDS_glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    glVertex3v16(floattov16(x), floattov16(y), floattov16(z));
    gNDS_GLState.vertexCount++;
}

void NDS_glVertex3fv(const GLfloat* v)
{
    glVertex3v16(floattov16(v[0]), floattov16(v[1]), floattov16(v[2]));
    gNDS_GLState.vertexCount++;
}

void NDS_glNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
    gNDS_GLState.currentNormal[0] = x;
    gNDS_GLState.currentNormal[1] = y;
    gNDS_GLState.currentNormal[2] = z;

    // NDS normals use v10 format (1.9 fixed point, range -1 to ~1)
    glNormal3f(x, y, z);
}

void NDS_glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
    gNDS_GLState.currentColor[0] = r;
    gNDS_GLState.currentColor[1] = g;
    gNDS_GLState.currentColor[2] = b;
    gNDS_GLState.currentColor[3] = 1.0f;

    glColor3b((uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
}

void NDS_glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    gNDS_GLState.currentColor[0] = r;
    gNDS_GLState.currentColor[1] = g;
    gNDS_GLState.currentColor[2] = b;
    gNDS_GLState.currentColor[3] = a;

    glColor3b((uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
    // NDS doesn't have per-vertex alpha in the same way; alpha is per-polygon
}

void NDS_glColor4fv(const GLfloat* v)
{
    NDS_glColor4f(v[0], v[1], v[2], v[3]);
}

void NDS_glTexCoord2f(GLfloat u, GLfloat v)
{
    gNDS_GLState.currentTexCoord[0] = u;
    gNDS_GLState.currentTexCoord[1] = v;

    // NDS texture coordinates use t16 format (12.4 fixed point)
    glTexCoord2t16(floattot16(u), floattot16(v));
}

void NDS_glTexCoord2fv(const GLfloat* v)
{
    NDS_glTexCoord2f(v[0], v[1]);
}

/****************************/
/*    TEXTURE MANAGEMENT    */
/****************************/

void NDS_glGenTextures(GLsizei n, GLuint* textures)
{
    for (int i = 0; i < n; i++)
    {
        textures[i] = gNDS_NextTextureID++;
        if (textures[i] < NDS_MAX_TEXTURES)
        {
            gNDS_Textures[textures[i]].used = 1;
            // Generate an NDS texture
            glGenTextures(1, &gNDS_Textures[textures[i]].ndsTextureID);
        }
    }
}

void NDS_glDeleteTextures(GLsizei n, const GLuint* textures)
{
    for (int i = 0; i < n; i++)
    {
        GLuint id = textures[i];
        if (id < NDS_MAX_TEXTURES && gNDS_Textures[id].used)
        {
            glDeleteTextures(1, (GLuint*)&gNDS_Textures[id].ndsTextureID);
            gNDS_Textures[id].used = 0;
        }
    }
}

void NDS_glBindTexture(GLenum target, GLuint texture)
{
    (void)target;
    gNDS_GLState.currentTexture = texture;

    if (texture == 0 || texture >= NDS_MAX_TEXTURES || !gNDS_Textures[texture].used)
    {
        // Unbind
        glBindTexture(0, 0);
        return;
    }

    glBindTexture(0, gNDS_Textures[texture].ndsTextureID);
}

// Helper to find the closest power-of-2 NDS texture size
static int NDS_GetTextureSizeEnum(int size)
{
    if (size <= 8)   return TEXTURE_SIZE_8;
    if (size <= 16)  return TEXTURE_SIZE_16;
    if (size <= 32)  return TEXTURE_SIZE_32;
    if (size <= 64)  return TEXTURE_SIZE_64;
    if (size <= 128) return TEXTURE_SIZE_128;
    if (size <= 256) return TEXTURE_SIZE_256;
    if (size <= 512) return TEXTURE_SIZE_512;
    return TEXTURE_SIZE_1024;
}

void NDS_glTexImage2D(GLenum target, GLint level, GLint internalformat,
                      GLsizei width, GLsizei height, GLint border,
                      GLenum format, GLenum type, const void* data)
{
    (void)target; (void)level; (void)border;

    GLuint texID = gNDS_GLState.currentTexture;
    if (texID == 0 || texID >= NDS_MAX_TEXTURES || !gNDS_Textures[texID].used)
        return;

    gNDS_Textures[texID].width = width;
    gNDS_Textures[texID].height = height;

    // Convert RGBA data to NDS RGB15A1 format
    // NDS supports several texture formats; we'll use TEXGEN_TEXCOORD with direct color (A1RGB5)
    int numPixels = width * height;
    uint16_t* ndsData = (uint16_t*)malloc(numPixels * sizeof(uint16_t));
    if (!ndsData)
        return;

    if (data)
    {
        const uint8_t* src = (const uint8_t*)data;
        int srcBpp = (format == GL_RGBA) ? 4 : 3;

        for (int i = 0; i < numPixels; i++)
        {
            uint8_t r = src[i * srcBpp + 0] >> 3;
            uint8_t g = src[i * srcBpp + 1] >> 3;
            uint8_t b = src[i * srcBpp + 2] >> 3;
            uint8_t a = 1;
            if (srcBpp == 4 && src[i * srcBpp + 3] < 128)
                a = 0;

            ndsData[i] = ARGB16(a, r, g, b);
        }
    }
    else
    {
        memset(ndsData, 0, numPixels * sizeof(uint16_t));
    }

    int sizeW = NDS_GetTextureSizeEnum(width);
    int sizeH = NDS_GetTextureSizeEnum(height);

    glTexImage2D(0, 0, GL_RGB, sizeW, sizeH, 0, TEXGEN_TEXCOORD, (uint8_t*)ndsData);

    free(ndsData);
}

void NDS_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    // NDS texture parameters are limited; store for reference
    (void)target; (void)pname; (void)param;
}

void NDS_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    (void)target; (void)pname; (void)param;
}

void NDS_glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    // NDS has limited texture environment support
    (void)target; (void)pname; (void)param;
}

void NDS_glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    // Texture coordinate generation - limited support on NDS
    (void)coord; (void)pname; (void)param;
}

/****************************/
/*    LIGHTING              */
/****************************/

void NDS_glLightfv(GLenum light, GLenum pname, const GLfloat* params)
{
    int lightIndex = light - GL_LIGHT0;
    if (lightIndex < 0 || lightIndex > 3)
        return;

    switch (pname)
    {
        case GL_POSITION:
        {
            // NDS lights are directional only (no positional lights)
            // Normalize the direction
            float x = params[0], y = params[1], z = params[2];
            float len = sqrtf(x*x + y*y + z*z);
            if (len > 0.001f)
            {
                x /= len; y /= len; z /= len;
            }
            glLight(lightIndex, NDS_FloatColorToRGB15(
                gNDS_GLState.currentColor[0],
                gNDS_GLState.currentColor[1],
                gNDS_GLState.currentColor[2]),
                floattov10(x), floattov10(y), floattov10(z));
            break;
        }
        case GL_DIFFUSE:
        {
            uint16_t color = NDS_FloatColorToRGB15(params[0], params[1], params[2]);
            glLight(lightIndex, color,
                floattov10(0), floattov10(0), floattov10(-1)); // Default direction
            break;
        }
        case GL_AMBIENT:
        case GL_SPECULAR:
            // Tracked but limited hardware support
            break;
    }
}

void NDS_glLightModelfv(GLenum pname, const GLfloat* params)
{
    if (pname == GL_LIGHT_MODEL_AMBIENT)
    {
        // Set ambient color
        // NDS doesn't have a separate ambient light model, but we can use material emission
        (void)params;
    }
}

void NDS_glMaterialfv(GLenum face, GLenum pname, const GLfloat* params)
{
    (void)face;

    switch (pname)
    {
        case GL_AMBIENT:
            gNDS_GLState.materialAmbient[0] = params[0];
            gNDS_GLState.materialAmbient[1] = params[1];
            gNDS_GLState.materialAmbient[2] = params[2];
            gNDS_GLState.materialAmbient[3] = params[3];
            glMaterialf(GL_AMBIENT, NDS_FloatColorToRGB15(params[0], params[1], params[2]));
            break;
        case GL_DIFFUSE:
            gNDS_GLState.materialDiffuse[0] = params[0];
            gNDS_GLState.materialDiffuse[1] = params[1];
            gNDS_GLState.materialDiffuse[2] = params[2];
            gNDS_GLState.materialDiffuse[3] = params[3];
            glMaterialf(GL_DIFFUSE, NDS_FloatColorToRGB15(params[0], params[1], params[2]));
            break;
        case GL_SPECULAR:
            gNDS_GLState.materialSpecular[0] = params[0];
            gNDS_GLState.materialSpecular[1] = params[1];
            gNDS_GLState.materialSpecular[2] = params[2];
            gNDS_GLState.materialSpecular[3] = params[3];
            glMaterialf(GL_SPECULAR, NDS_FloatColorToRGB15(params[0], params[1], params[2]));
            break;
        case GL_EMISSION:
            gNDS_GLState.materialEmission[0] = params[0];
            gNDS_GLState.materialEmission[1] = params[1];
            gNDS_GLState.materialEmission[2] = params[2];
            gNDS_GLState.materialEmission[3] = params[3];
            glMaterialf(GL_EMISSION, NDS_FloatColorToRGB15(params[0], params[1], params[2]));
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            gNDS_GLState.materialAmbient[0] = params[0];
            gNDS_GLState.materialAmbient[1] = params[1];
            gNDS_GLState.materialAmbient[2] = params[2];
            gNDS_GLState.materialAmbient[3] = params[3];
            gNDS_GLState.materialDiffuse[0] = params[0];
            gNDS_GLState.materialDiffuse[1] = params[1];
            gNDS_GLState.materialDiffuse[2] = params[2];
            gNDS_GLState.materialDiffuse[3] = params[3];
            glMaterialf(GL_AMBIENT, NDS_FloatColorToRGB15(params[0], params[1], params[2]));
            glMaterialf(GL_DIFFUSE, NDS_FloatColorToRGB15(params[0], params[1], params[2]));
            break;
    }
}

/****************************/
/*    FOG                   */
/****************************/

void NDS_glFogf(GLenum pname, GLfloat param)
{
    switch (pname)
    {
        case GL_FOG_START:      gNDS_GLState.fogStart = param; break;
        case GL_FOG_END:        gNDS_GLState.fogEnd = param; break;
        case GL_FOG_DENSITY:    gNDS_GLState.fogDensity = param; break;
    }
    // NDS fog is set via glFogShift and glFogColor, handled differently
}

void NDS_glFogfv(GLenum pname, const GLfloat* params)
{
    if (pname == GL_FOG_COLOR)
    {
        gNDS_GLState.fogColor[0] = params[0];
        gNDS_GLState.fogColor[1] = params[1];
        gNDS_GLState.fogColor[2] = params[2];
        gNDS_GLState.fogColor[3] = params[3];
    }
}

void NDS_glFogi(GLenum pname, GLint param)
{
    if (pname == GL_FOG_MODE)
    {
        gNDS_GLState.fogMode = param;
    }
}

/****************************/
/*    ALPHA TEST            */
/****************************/

void NDS_glAlphaFunc(GLenum func, GLfloat ref)
{
    gNDS_GLState.alphaFunc = func;
    gNDS_GLState.alphaRef = ref;
    // NDS alpha test is per-polygon via polygon attributes
}

/****************************/
/*    BLEND                 */
/****************************/

void NDS_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    gNDS_GLState.blendSrc = sfactor;
    gNDS_GLState.blendDst = dfactor;
    // NDS blending is handled via polygon alpha (0-31)
}

/****************************/
/*    DEPTH                 */
/****************************/

void NDS_glDepthMask(GLboolean flag)
{
    gNDS_GLState.depthMaskEnabled = flag;
    // NDS depth write is controlled per-polygon
}

/****************************/
/*    COLOR MASK            */
/****************************/

void NDS_glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    // NDS doesn't support per-channel color masking
    (void)r; (void)g; (void)b; (void)a;
}

/****************************/
/*    VERTEX ARRAYS         */
/****************************/

void NDS_glEnableClientState(GLenum array)
{
    switch (array)
    {
        case GL_VERTEX_ARRAY:       gNDS_VertexArrayState.vertexArrayEnabled = 1; break;
        case GL_NORMAL_ARRAY:       gNDS_VertexArrayState.normalArrayEnabled = 1; break;
        case GL_COLOR_ARRAY:        gNDS_VertexArrayState.colorArrayEnabled = 1; break;
        case GL_TEXTURE_COORD_ARRAY: gNDS_VertexArrayState.texCoordArrayEnabled = 1; break;
    }
}

void NDS_glDisableClientState(GLenum array)
{
    switch (array)
    {
        case GL_VERTEX_ARRAY:       gNDS_VertexArrayState.vertexArrayEnabled = 0; break;
        case GL_NORMAL_ARRAY:       gNDS_VertexArrayState.normalArrayEnabled = 0; break;
        case GL_COLOR_ARRAY:        gNDS_VertexArrayState.colorArrayEnabled = 0; break;
        case GL_TEXTURE_COORD_ARRAY: gNDS_VertexArrayState.texCoordArrayEnabled = 0; break;
    }
}

void NDS_glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    gNDS_VertexArrayState.vertexPointer = (const GLfloat*)pointer;
    gNDS_VertexArrayState.vertexSize = size;
    gNDS_VertexArrayState.vertexStride = stride ? stride : (size * sizeof(GLfloat));
}

void NDS_glNormalPointer(GLenum type, GLsizei stride, const void* pointer)
{
    (void)type;
    gNDS_VertexArrayState.normalPointer = (const GLfloat*)pointer;
    gNDS_VertexArrayState.normalStride = stride ? stride : (3 * sizeof(GLfloat));
}

void NDS_glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    (void)type;
    gNDS_VertexArrayState.colorPointer = (const GLfloat*)pointer;
    gNDS_VertexArrayState.colorSize = size;
    gNDS_VertexArrayState.colorStride = stride ? stride : (size * sizeof(GLfloat));
}

void NDS_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    (void)type;
    gNDS_VertexArrayState.texCoordPointer = (const GLfloat*)pointer;
    gNDS_VertexArrayState.texCoordSize = size;
    gNDS_VertexArrayState.texCoordStride = stride ? stride : (size * sizeof(GLfloat));
}

// Helper to submit a single vertex from array data
static void NDS_SubmitArrayVertex(int index)
{
    // Normals
    if (gNDS_VertexArrayState.normalArrayEnabled && gNDS_VertexArrayState.normalPointer)
    {
        const uint8_t* base = (const uint8_t*)gNDS_VertexArrayState.normalPointer;
        const GLfloat* n = (const GLfloat*)(base + index * gNDS_VertexArrayState.normalStride);
        glNormal3f(n[0], n[1], n[2]);
    }

    // Colors
    if (gNDS_VertexArrayState.colorArrayEnabled && gNDS_VertexArrayState.colorPointer)
    {
        const uint8_t* base = (const uint8_t*)gNDS_VertexArrayState.colorPointer;
        const GLfloat* c = (const GLfloat*)(base + index * gNDS_VertexArrayState.colorStride);
        glColor3b((uint8_t)(c[0] * 255.0f), (uint8_t)(c[1] * 255.0f), (uint8_t)(c[2] * 255.0f));
    }

    // Texture coordinates
    if (gNDS_VertexArrayState.texCoordArrayEnabled && gNDS_VertexArrayState.texCoordPointer)
    {
        const uint8_t* base = (const uint8_t*)gNDS_VertexArrayState.texCoordPointer;
        const GLfloat* t = (const GLfloat*)(base + index * gNDS_VertexArrayState.texCoordStride);
        glTexCoord2t16(floattot16(t[0]), floattot16(t[1]));
    }

    // Position (must be last for NDS hardware)
    if (gNDS_VertexArrayState.vertexArrayEnabled && gNDS_VertexArrayState.vertexPointer)
    {
        const uint8_t* base = (const uint8_t*)gNDS_VertexArrayState.vertexPointer;
        const GLfloat* v = (const GLfloat*)(base + index * gNDS_VertexArrayState.vertexStride);
        if (gNDS_VertexArrayState.vertexSize >= 3)
            glVertex3v16(floattov16(v[0]), floattov16(v[1]), floattov16(v[2]));
        else
            glVertex3v16(floattov16(v[0]), floattov16(v[1]), 0);
    }

    gNDS_GLState.vertexCount++;
}

void NDS_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
    // Convert GL mode to NDS mode
    GLenum ndsMode;
    switch (mode)
    {
        case GL_TRIANGLES:      ndsMode = GL_TRIANGLES; break;
        case GL_TRIANGLE_STRIP: ndsMode = GL_TRIANGLE_STRIP; break;
        case GL_QUADS:          ndsMode = GL_QUADS; break;
        case GL_QUAD_STRIP:     ndsMode = GL_QUAD_STRIP; break;
        default:                ndsMode = GL_TRIANGLES; break;
    }

    glBegin(ndsMode);

    for (int i = 0; i < count; i++)
    {
        int index;
        if (type == GL_UNSIGNED_SHORT)
            index = ((const GLushort*)indices)[i];
        else if (type == GL_UNSIGNED_BYTE)
            index = ((const GLubyte*)indices)[i];
        else
            index = ((const GLuint*)indices)[i];

        NDS_SubmitArrayVertex(index);
    }

    glEnd();
    gNDS_GLState.polyCount += count / 3; // Approximate
}

void NDS_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    GLenum ndsMode;
    switch (mode)
    {
        case GL_TRIANGLES:      ndsMode = GL_TRIANGLES; break;
        case GL_TRIANGLE_STRIP: ndsMode = GL_TRIANGLE_STRIP; break;
        case GL_QUADS:          ndsMode = GL_QUADS; break;
        case GL_QUAD_STRIP:     ndsMode = GL_QUAD_STRIP; break;
        default:                ndsMode = GL_TRIANGLES; break;
    }

    glBegin(ndsMode);

    for (int i = 0; i < count; i++)
    {
        NDS_SubmitArrayVertex(first + i);
    }

    glEnd();
    gNDS_GLState.polyCount += count / 3;
}

/****************************/
/*    MULTI-TEXTURE         */
/****************************/

void NDS_glActiveTexture(GLenum texture)
{
    gNDS_GLState.activeTextureUnit = texture - GL_TEXTURE0;
    // NDS only has one texture unit in hardware
}

void NDS_glClientActiveTexture(GLenum texture)
{
    gNDS_VertexArrayState.activeClientTexture = texture - GL_TEXTURE0;
}

/****************************/
/*    STATE QUERIES         */
/****************************/

void NDS_glGetFloatv(GLenum pname, GLfloat* params)
{
    switch (pname)
    {
        case GL_MODELVIEW_MATRIX:
        {
            // Return identity as fallback - NDS matrices are fixed-point
            float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
            memcpy(params, identity, sizeof(identity));
            break;
        }
        case GL_PROJECTION_MATRIX:
        {
            float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
            memcpy(params, identity, sizeof(identity));
            break;
        }
        default:
            break;
    }
}

void NDS_glGetIntegerv(GLenum pname, GLint* params)
{
    switch (pname)
    {
        case GL_VIEWPORT:
            params[0] = 0;
            params[1] = 0;
            params[2] = NDS_SCREEN_WIDTH;
            params[3] = NDS_SCREEN_HEIGHT;
            break;
        default:
            break;
    }
}

void NDS_glGetBooleanv(GLenum pname, GLboolean* params)
{
    (void)pname;
    params[0] = 0;
}

const GLubyte* NDS_glGetString(GLenum name)
{
    switch (name)
    {
        case GL_VENDOR:     return (const GLubyte*)"Nintendo DS (libnds)";
        case GL_RENDERER:   return (const GLubyte*)"NDS 3D Engine";
        case GL_VERSION:    return (const GLubyte*)"1.0 NDS";
        case GL_EXTENSIONS: return (const GLubyte*)"";
        default:            return (const GLubyte*)"";
    }
}

GLenum NDS_glGetError(void)
{
    return GL_NO_ERROR;
}

/****************************/
/*    MISC                  */
/****************************/

void NDS_glHint(GLenum target, GLenum mode)
{
    (void)target; (void)mode;
}

void NDS_glPixelStorei(GLenum pname, GLint param)
{
    (void)pname; (void)param;
}

void NDS_glLineWidth(GLfloat width)
{
    (void)width;
}

void NDS_glFrontFace(GLenum mode)
{
    (void)mode;
}

void NDS_glCullFace(GLenum mode)
{
    (void)mode;
    // NDS cull face is handled via polygon attributes
}

void NDS_glPolygonMode(GLenum face, GLenum mode)
{
    (void)face; (void)mode;
    // NDS doesn't support wireframe rendering
}

/****************************/
/*    BUFFER STUBS          */
/****************************/

void NDS_glGenBuffers(GLsizei n, GLuint* buffers)
{
    for (int i = 0; i < n; i++)
        buffers[i] = gNDS_NextBufferID++;
}

void NDS_glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    (void)n; (void)buffers;
}

void NDS_glBindBuffer(GLenum target, GLuint buffer)
{
    (void)target; (void)buffer;
}

void NDS_glBufferData(GLenum target, GLsizei size, const void* data, GLenum usage)
{
    (void)target; (void)size; (void)data; (void)usage;
}

/****************************/
/*    SHADER STUBS          */
/****************************/
// NDS uses fixed-function pipeline; all shader functions are no-ops

GLuint NDS_glCreateShader(GLenum type) { (void)type; return 1; }
void NDS_glShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length) { (void)shader; (void)count; (void)string; (void)length; }
void NDS_glCompileShader(GLuint shader) { (void)shader; }
void NDS_glGetShaderiv(GLuint shader, GLenum pname, GLint* params) { (void)shader; (void)pname; if (params) *params = 1; }
void NDS_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, char* infoLog) { (void)shader; (void)maxLength; if (length) *length = 0; if (infoLog) infoLog[0] = '\0'; }
GLuint NDS_glCreateProgram(void) { return 1; }
void NDS_glAttachShader(GLuint program, GLuint shader) { (void)program; (void)shader; }
void NDS_glBindAttribLocation(GLuint program, GLuint index, const char* name) { (void)program; (void)index; (void)name; }
void NDS_glLinkProgram(GLuint program) { (void)program; }
void NDS_glGetProgramiv(GLuint program, GLenum pname, GLint* params) { (void)program; (void)pname; if (params) *params = 1; }
void NDS_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei* length, char* infoLog) { (void)program; (void)maxLength; if (length) *length = 0; if (infoLog) infoLog[0] = '\0'; }
void NDS_glValidateProgram(GLuint program) { (void)program; }
void NDS_glUseProgram(GLuint program) { (void)program; }
GLint NDS_glGetUniformLocation(GLuint program, const char* name) { (void)program; (void)name; return -1; }
void NDS_glUniform1f(GLint location, GLfloat v0) { (void)location; (void)v0; }
void NDS_glUniform1i(GLint location, GLint v0) { (void)location; (void)v0; }
void NDS_glUniform3fv(GLint location, GLsizei count, const GLfloat* value) { (void)location; (void)count; (void)value; }
void NDS_glUniform4fv(GLint location, GLsizei count, const GLfloat* value) { (void)location; (void)count; (void)value; }
void NDS_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { (void)location; (void)count; (void)transpose; (void)value; }
void NDS_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { (void)location; (void)count; (void)transpose; (void)value; }
void NDS_glEnableVertexAttribArray(GLuint index) { (void)index; }
void NDS_glDisableVertexAttribArray(GLuint index) { (void)index; }
void NDS_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) { (void)index; (void)size; (void)type; (void)normalized; (void)stride; (void)pointer; }

#endif // NDS
