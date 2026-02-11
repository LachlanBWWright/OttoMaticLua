/****************************/
/*   GLES 3.0 BRIDGE        */
/*   Fixed-function pipeline */
/*   emulation via shaders   */
/****************************/

#ifdef __ANDROID__

#include "gles_bridge.h"
#include <stdio.h>

// ============================================================================
// Shader source code
// ============================================================================

static const char* sVertexShaderSource =
    "#version 300 es\n"
    "precision highp float;\n"
    "\n"
    "in vec4 a_position;\n"
    "in vec3 a_normal;\n"
    "in vec2 a_texcoord;\n"
    "in vec4 a_color;\n"
    "in vec2 a_texcoord1;\n"
    "\n"
    "uniform mat4 u_projection;\n"
    "uniform mat4 u_modelview;\n"
    "uniform mat3 u_normalMatrix;\n"
    "\n"
    "uniform bool u_lightingEnabled;\n"
    "uniform vec4 u_ambientLight;\n"
    "uniform bool u_light0Enabled;\n"
    "uniform vec4 u_light0Position;\n"
    "uniform vec4 u_light0Diffuse;\n"
    "uniform bool u_light1Enabled;\n"
    "uniform vec4 u_light1Position;\n"
    "uniform vec4 u_light1Diffuse;\n"
    "uniform bool u_light2Enabled;\n"
    "uniform vec4 u_light2Position;\n"
    "uniform vec4 u_light2Diffuse;\n"
    "uniform bool u_light3Enabled;\n"
    "uniform vec4 u_light3Position;\n"
    "uniform vec4 u_light3Diffuse;\n"
    "\n"
    "uniform bool u_fogEnabled;\n"
    "uniform float u_fogStart;\n"
    "uniform float u_fogEnd;\n"
    "\n"
    "// Texture generation (sphere mapping)\n"
    "uniform bool u_texGenSEnabled;\n"
    "uniform bool u_texGenTEnabled;\n"
    "\n"
    "out vec4 v_color;\n"
    "out vec2 v_texcoord;\n"
    "out vec2 v_texcoord1;\n"
    "out float v_fogFactor;\n"
    "\n"
    "void main() {\n"
    "    vec4 eyePos = u_modelview * a_position;\n"
    "    gl_Position = u_projection * eyePos;\n"
    "    v_texcoord = a_texcoord;\n"
    "\n"
    "    // Compute sphere map texcoords for texture unit 1 if texgen enabled\n"
    "    if (u_texGenSEnabled || u_texGenTEnabled) {\n"
    "        vec3 eyeNorm = normalize(u_normalMatrix * a_normal);\n"
    "        vec3 eyeDir = normalize(eyePos.xyz);\n"
    "        vec3 r = reflect(eyeDir, eyeNorm);\n"
    "        float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0));\n"
    "        v_texcoord1 = vec2(r.x/m + 0.5, r.y/m + 0.5);\n"
    "    } else {\n"
    "        v_texcoord1 = a_texcoord1;\n"
    "    }\n"
    "\n"
    "    if (u_lightingEnabled) {\n"
    "        vec3 normal = normalize(u_normalMatrix * a_normal);\n"
    "        vec4 litColor = u_ambientLight * a_color;\n"
    "        if (u_light0Enabled) {\n"
    "            float ndotl = max(dot(normal, normalize(u_light0Position.xyz)), 0.0);\n"
    "            litColor += u_light0Diffuse * a_color * ndotl;\n"
    "        }\n"
    "        if (u_light1Enabled) {\n"
    "            float ndotl = max(dot(normal, normalize(u_light1Position.xyz)), 0.0);\n"
    "            litColor += u_light1Diffuse * a_color * ndotl;\n"
    "        }\n"
    "        if (u_light2Enabled) {\n"
    "            float ndotl = max(dot(normal, normalize(u_light2Position.xyz)), 0.0);\n"
    "            litColor += u_light2Diffuse * a_color * ndotl;\n"
    "        }\n"
    "        if (u_light3Enabled) {\n"
    "            float ndotl = max(dot(normal, normalize(u_light3Position.xyz)), 0.0);\n"
    "            litColor += u_light3Diffuse * a_color * ndotl;\n"
    "        }\n"
    "        litColor.a = a_color.a;\n"
    "        v_color = clamp(litColor, 0.0, 1.0);\n"
    "    } else {\n"
    "        v_color = a_color;\n"
    "    }\n"
    "\n"
    "    if (u_fogEnabled) {\n"
    "        float dist = length(eyePos.xyz);\n"
    "        v_fogFactor = clamp((u_fogEnd - dist) / (u_fogEnd - u_fogStart), 0.0, 1.0);\n"
    "    } else {\n"
    "        v_fogFactor = 1.0;\n"
    "    }\n"
    "}\n";

static const char* sFragmentShaderSource =
    "#version 300 es\n"
    "precision mediump float;\n"
    "\n"
    "in vec4 v_color;\n"
    "in vec2 v_texcoord;\n"
    "in vec2 v_texcoord1;\n"
    "in float v_fogFactor;\n"
    "\n"
    "uniform bool u_textureEnabled;\n"
    "uniform sampler2D u_texture0;\n"
    "uniform bool u_texture1Enabled;\n"
    "uniform sampler2D u_texture1;\n"
    "uniform int u_texEnvMode1;\n"  // 0=modulate, 1=add, 2=replace
    "uniform bool u_fogEnabled;\n"
    "uniform vec4 u_fogColor;\n"
    "uniform bool u_alphaTestEnabled;\n"
    "uniform float u_alphaRef;\n"
    "uniform int u_alphaFunc;\n"
    "\n"
    "out vec4 fragColor;\n"
    "\n"
    "void main() {\n"
    "    vec4 color;\n"
    "    if (u_textureEnabled) {\n"
    "        color = texture(u_texture0, v_texcoord) * v_color;\n"
    "    } else {\n"
    "        color = v_color;\n"
    "    }\n"
    "\n"
    "    // Multi-texture: blend texture unit 1 on top\n"
    "    if (u_texture1Enabled) {\n"
    "        vec4 tex1 = texture(u_texture1, v_texcoord1);\n"
    "        if (u_texEnvMode1 == 1) {\n"  // ADD
    "            color.rgb = color.rgb + tex1.rgb;\n"
    "        } else if (u_texEnvMode1 == 2) {\n"  // REPLACE
    "            color.rgb = tex1.rgb;\n"
    "        } else {\n"  // MODULATE (default)
    "            color.rgb = color.rgb * tex1.rgb;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    if (u_alphaTestEnabled) {\n"
    "        bool pass = false;\n"
    "        if (u_alphaFunc == 0x0205) { pass = (color.a != u_alphaRef); }\n"  // GL_NOTEQUAL
    "        else if (u_alphaFunc == 0x0201) { pass = (color.a < u_alphaRef); }\n"  // GL_LESS
    "        else if (u_alphaFunc == 0x0204) { pass = (color.a > u_alphaRef); }\n"  // GL_GREATER
    "        else if (u_alphaFunc == 0x0203) { pass = (color.a >= u_alphaRef); }\n" // GL_GEQUAL
    "        else if (u_alphaFunc == 0x0202) { pass = (color.a <= u_alphaRef); }\n" // GL_LEQUAL
    "        else if (u_alphaFunc == 0x0200) { pass = false; }\n" // GL_NEVER
    "        else if (u_alphaFunc == 0x0207) { pass = true; }\n"  // GL_ALWAYS
    "        else { pass = (color.a != u_alphaRef); }\n"
    "        if (!pass) discard;\n"
    "    }\n"
    "\n"
    "    if (u_fogEnabled) {\n"
    "        color.rgb = mix(u_fogColor.rgb, color.rgb, v_fogFactor);\n"
    "    }\n"
    "\n"
    "    fragColor = color;\n"
    "}\n";

// ============================================================================
// Attribute locations (fixed)
// ============================================================================

#define ATTR_POSITION  0
#define ATTR_NORMAL    1
#define ATTR_TEXCOORD  2
#define ATTR_COLOR     3
#define ATTR_TEXCOORD1 4

// ============================================================================
// Global state
// ============================================================================

static GLuint sShaderProgram = 0;
static GLuint sVAO = 0;

// Uniform locations
static GLint sLoc_projection = -1;
static GLint sLoc_modelview = -1;
static GLint sLoc_normalMatrix = -1;
static GLint sLoc_lightingEnabled = -1;
static GLint sLoc_ambientLight = -1;
static GLint sLoc_light0Enabled = -1;
static GLint sLoc_light0Position = -1;
static GLint sLoc_light0Diffuse = -1;
static GLint sLoc_light1Enabled = -1;
static GLint sLoc_light1Position = -1;
static GLint sLoc_light1Diffuse = -1;
static GLint sLoc_light2Enabled = -1;
static GLint sLoc_light2Position = -1;
static GLint sLoc_light2Diffuse = -1;
static GLint sLoc_light3Enabled = -1;
static GLint sLoc_light3Position = -1;
static GLint sLoc_light3Diffuse = -1;
static GLint sLoc_fogEnabled = -1;
static GLint sLoc_fogStart = -1;
static GLint sLoc_fogEnd = -1;
static GLint sLoc_fogColor = -1;
static GLint sLoc_textureEnabled = -1;
static GLint sLoc_texture0 = -1;
static GLint sLoc_alphaTestEnabled = -1;
static GLint sLoc_alphaRef = -1;
static GLint sLoc_alphaFunc = -1;

// Multi-texture & texgen uniform locations
static GLint sLoc_texture1Enabled = -1;
static GLint sLoc_texture1 = -1;
static GLint sLoc_texEnvMode1 = -1;
static GLint sLoc_texGenSEnabled = -1;
static GLint sLoc_texGenTEnabled = -1;

// Matrix stacks
static BridgeMatrixStack sModelviewStack;
static BridgeMatrixStack sProjectionStack;
static GLenum sCurrentMatrixMode = GL_MODELVIEW;

// Current state
static GLfloat sCurrentColor[4] = {1, 1, 1, 1};
static GLfloat sCurrentNormal[3] = {0, 0, 1};
static GLfloat sCurrentTexCoord[2] = {0, 0};

// Lighting state
static GLboolean sLightingEnabled = GL_FALSE;
static GLfloat sAmbientLight[4] = {0.2f, 0.2f, 0.2f, 1.0f};
static BridgeLightState sLights[BRIDGE_MAX_LIGHTS];

// Fog state
static GLboolean sFogEnabled = GL_FALSE;
static GLfloat sFogStart = 0.0f;
static GLfloat sFogEnd = 1.0f;
static GLfloat sFogColor[4] = {0, 0, 0, 1};
static GLint sFogMode = 0x2601; // GL_LINEAR

// Texture state
static GLboolean sTexture2DEnabled = GL_FALSE;
static GLboolean sNormalizeEnabled = GL_FALSE;

// Alpha test state
static GLboolean sAlphaTestEnabled = GL_FALSE;
static GLenum sAlphaTestFunc = GL_NOTEQUAL;
static GLfloat sAlphaTestRef = 0.0f;

// Vertex array state
static GLboolean sVertexArrayEnabled = GL_FALSE;
static GLboolean sNormalArrayEnabled = GL_FALSE;
static GLboolean sColorArrayEnabled = GL_FALSE;
static GLboolean sTexCoordArrayEnabled = GL_FALSE;

static GLint sVertexArraySize = 3;
static GLenum sVertexArrayType = GL_FLOAT;
static GLsizei sVertexArrayStride = 0;
static const void* sVertexArrayPtr = NULL;

static GLenum sNormalArrayType = GL_FLOAT;
static GLsizei sNormalArrayStride = 0;
static const void* sNormalArrayPtr = NULL;

static GLint sColorArraySize = 4;
static GLenum sColorArrayType = GL_FLOAT;
static GLsizei sColorArrayStride = 0;
static const void* sColorArrayPtr = NULL;

static GLint sTexCoordArraySize = 2;
static GLenum sTexCoordArrayType = GL_FLOAT;
static GLsizei sTexCoordArrayStride = 0;
static const void* sTexCoordArrayPtr = NULL;

// Active client texture unit
static GLenum sClientActiveTexture = GL_TEXTURE0;

// Active server texture unit (for glEnable(GL_TEXTURE_2D) per unit)
static GLenum sActiveTexture = GL_TEXTURE0;

// Texture unit 1 state for multi-texturing
static GLboolean sTexture1Enabled = GL_FALSE;
static GLint sTexEnvMode1 = 0;  // 0=modulate, 1=add, 2=replace

// Texture unit 1 coord arrays
static GLboolean sTexCoord1ArrayEnabled = GL_FALSE;
static GLint sTexCoord1ArraySize = 2;
static GLenum sTexCoord1ArrayType = GL_FLOAT;
static GLsizei sTexCoord1ArrayStride = 0;
static const void* sTexCoord1ArrayPtr = NULL;

// Texture generation state (sphere mapping)
static GLboolean sTexGenSEnabled = GL_FALSE;
static GLboolean sTexGenTEnabled = GL_FALSE;

// Immediate mode state
static GLenum sImmMode = GL_TRIANGLES;
static GLboolean sInImmMode = GL_FALSE;

// Immediate mode vertex buffer: pos(3) + normal(3) + texcoord(2) + color(4) = 12 floats per vertex
#define IMM_FLOATS_PER_VERTEX 12
static GLfloat sImmVertices[BRIDGE_IMM_MAX_VERTICES * IMM_FLOATS_PER_VERTEX];
static int sImmVertexCount = 0;
static GLuint sImmVBO = 0;

// Blend state tracking
static GLint sBlendSrc = GL_ONE;
static GLint sBlendDst = GL_ZERO;

// Depth mask tracking
static GLboolean sDepthMask = GL_TRUE;

// ============================================================================
// Helper: identity matrix
// ============================================================================

static void Mat4Identity(GLfloat *m)
{
    memset(m, 0, 16 * sizeof(GLfloat));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// ============================================================================
// Helper: multiply two 4x4 matrices (column-major) result = a * b
// ============================================================================

static void Mat4Multiply(const GLfloat *a, const GLfloat *b, GLfloat *result)
{
    GLfloat tmp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            tmp[j*4+i] = 0;
            for (int k = 0; k < 4; k++) {
                tmp[j*4+i] += a[k*4+i] * b[j*4+k];
            }
        }
    }
    memcpy(result, tmp, 16 * sizeof(GLfloat));
}

// ============================================================================
// Helper: extract 3x3 normal matrix from 4x4 modelview (upper-left)
// ============================================================================

static void ExtractNormalMatrix(const GLfloat *mv, GLfloat *nm)
{
    // For normal transformation, we need the inverse-transpose of the upper-left 3x3.
    // For orthonormal modelview matrices (no non-uniform scaling), upper-left 3x3 suffices.
    nm[0] = mv[0]; nm[1] = mv[1]; nm[2] = mv[2];
    nm[3] = mv[4]; nm[4] = mv[5]; nm[5] = mv[6];
    nm[6] = mv[8]; nm[7] = mv[9]; nm[8] = mv[10];
}

// ============================================================================
// Shader compilation
// ============================================================================

static GLuint CompileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        BRIDGE_LOGE("Shader compile error: %s", infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint CreateProgram(const char *vertSrc, const char *fragSrc)
{
    GLuint vert = CompileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = CompileShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!vert || !frag) {
        BRIDGE_LOGE("Failed to compile shaders");
        return 0;
    }

    GLuint program = glCreateProgram();

    // Bind attribute locations before linking
    glBindAttribLocation(program, ATTR_POSITION, "a_position");
    glBindAttribLocation(program, ATTR_NORMAL, "a_normal");
    glBindAttribLocation(program, ATTR_TEXCOORD, "a_texcoord");
    glBindAttribLocation(program, ATTR_COLOR, "a_color");
    glBindAttribLocation(program, ATTR_TEXCOORD1, "a_texcoord1");

    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        BRIDGE_LOGE("Program link error: %s", infoLog);
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

// ============================================================================
// Initialization
// ============================================================================

void GLESBridge_Init(void)
{
    BRIDGE_LOGI("Initializing GLES 3.0 Bridge");

    // Create shader program
    sShaderProgram = CreateProgram(sVertexShaderSource, sFragmentShaderSource);
    if (!sShaderProgram) {
        BRIDGE_LOGE("FATAL: Failed to create shader program!");
        return;
    }

    // Get uniform locations
    sLoc_projection = glGetUniformLocation(sShaderProgram, "u_projection");
    sLoc_modelview = glGetUniformLocation(sShaderProgram, "u_modelview");
    sLoc_normalMatrix = glGetUniformLocation(sShaderProgram, "u_normalMatrix");
    sLoc_lightingEnabled = glGetUniformLocation(sShaderProgram, "u_lightingEnabled");
    sLoc_ambientLight = glGetUniformLocation(sShaderProgram, "u_ambientLight");
    sLoc_light0Enabled = glGetUniformLocation(sShaderProgram, "u_light0Enabled");
    sLoc_light0Position = glGetUniformLocation(sShaderProgram, "u_light0Position");
    sLoc_light0Diffuse = glGetUniformLocation(sShaderProgram, "u_light0Diffuse");
    sLoc_light1Enabled = glGetUniformLocation(sShaderProgram, "u_light1Enabled");
    sLoc_light1Position = glGetUniformLocation(sShaderProgram, "u_light1Position");
    sLoc_light1Diffuse = glGetUniformLocation(sShaderProgram, "u_light1Diffuse");
    sLoc_light2Enabled = glGetUniformLocation(sShaderProgram, "u_light2Enabled");
    sLoc_light2Position = glGetUniformLocation(sShaderProgram, "u_light2Position");
    sLoc_light2Diffuse = glGetUniformLocation(sShaderProgram, "u_light2Diffuse");
    sLoc_light3Enabled = glGetUniformLocation(sShaderProgram, "u_light3Enabled");
    sLoc_light3Position = glGetUniformLocation(sShaderProgram, "u_light3Position");
    sLoc_light3Diffuse = glGetUniformLocation(sShaderProgram, "u_light3Diffuse");
    sLoc_fogEnabled = glGetUniformLocation(sShaderProgram, "u_fogEnabled");
    sLoc_fogStart = glGetUniformLocation(sShaderProgram, "u_fogStart");
    sLoc_fogEnd = glGetUniformLocation(sShaderProgram, "u_fogEnd");
    sLoc_fogColor = glGetUniformLocation(sShaderProgram, "u_fogColor");
    sLoc_textureEnabled = glGetUniformLocation(sShaderProgram, "u_textureEnabled");
    sLoc_texture0 = glGetUniformLocation(sShaderProgram, "u_texture0");
    sLoc_alphaTestEnabled = glGetUniformLocation(sShaderProgram, "u_alphaTestEnabled");
    sLoc_alphaRef = glGetUniformLocation(sShaderProgram, "u_alphaRef");
    sLoc_alphaFunc = glGetUniformLocation(sShaderProgram, "u_alphaFunc");

    // Multi-texture & texgen uniforms
    sLoc_texture1Enabled = glGetUniformLocation(sShaderProgram, "u_texture1Enabled");
    sLoc_texture1 = glGetUniformLocation(sShaderProgram, "u_texture1");
    sLoc_texEnvMode1 = glGetUniformLocation(sShaderProgram, "u_texEnvMode1");
    sLoc_texGenSEnabled = glGetUniformLocation(sShaderProgram, "u_texGenSEnabled");
    sLoc_texGenTEnabled = glGetUniformLocation(sShaderProgram, "u_texGenTEnabled");

    // Initialize matrix stacks
    sModelviewStack.top = 0;
    sProjectionStack.top = 0;
    Mat4Identity(sModelviewStack.stack[0]);
    Mat4Identity(sProjectionStack.stack[0]);

    // Initialize lights
    for (int i = 0; i < BRIDGE_MAX_LIGHTS; i++) {
        sLights[i].enabled = GL_FALSE;
        sLights[i].position[0] = 0; sLights[i].position[1] = 0;
        sLights[i].position[2] = 1; sLights[i].position[3] = 0;
        sLights[i].ambient[0] = 0; sLights[i].ambient[1] = 0;
        sLights[i].ambient[2] = 0; sLights[i].ambient[3] = 1;
        sLights[i].diffuse[0] = 0; sLights[i].diffuse[1] = 0;
        sLights[i].diffuse[2] = 0; sLights[i].diffuse[3] = 1;
    }
    // Light 0 default diffuse is white
    sLights[0].diffuse[0] = 1; sLights[0].diffuse[1] = 1;
    sLights[0].diffuse[2] = 1; sLights[0].diffuse[3] = 1;

    // Create VAO for immediate mode
    glGenVertexArrays(1, &sVAO);

    // Create VBO for immediate mode
    glGenBuffers(1, &sImmVBO);

    // Use our shader
    glUseProgram(sShaderProgram);
    glUniform1i(sLoc_texture0, 0);  // texture unit 0
    glUniform1i(sLoc_texture1, 1);  // texture unit 1

    BRIDGE_LOGI("GLES 3.0 Bridge initialized successfully");
}

void GLESBridge_Shutdown(void)
{
    if (sShaderProgram) {
        glDeleteProgram(sShaderProgram);
        sShaderProgram = 0;
    }
    if (sVAO) {
        glDeleteVertexArrays(1, &sVAO);
        sVAO = 0;
    }
    if (sImmVBO) {
        glDeleteBuffers(1, &sImmVBO);
        sImmVBO = 0;
    }
}

// ============================================================================
// Get current matrix pointer
// ============================================================================

static GLfloat* GetCurrentMatrix(void)
{
    if (sCurrentMatrixMode == GL_PROJECTION)
        return sProjectionStack.stack[sProjectionStack.top];
    else
        return sModelviewStack.stack[sModelviewStack.top];
}

static BridgeMatrixStack* GetCurrentStack(void)
{
    if (sCurrentMatrixMode == GL_PROJECTION)
        return &sProjectionStack;
    else
        return &sModelviewStack;
}

// ============================================================================
// Matrix operations
// ============================================================================

void bridge_MatrixMode(GLenum mode)
{
    sCurrentMatrixMode = mode;
}

void bridge_PushMatrix(void)
{
    BridgeMatrixStack *stack = GetCurrentStack();
    if (stack->top >= BRIDGE_MATRIX_STACK_DEPTH - 1) {
        BRIDGE_LOGE("Matrix stack overflow!");
        return;
    }
    memcpy(stack->stack[stack->top + 1], stack->stack[stack->top], 16 * sizeof(GLfloat));
    stack->top++;
}

void bridge_PopMatrix(void)
{
    BridgeMatrixStack *stack = GetCurrentStack();
    if (stack->top <= 0) {
        BRIDGE_LOGE("Matrix stack underflow!");
        return;
    }
    stack->top--;
}

void bridge_LoadIdentity(void)
{
    Mat4Identity(GetCurrentMatrix());
}

void bridge_LoadMatrixf(const GLfloat *m)
{
    memcpy(GetCurrentMatrix(), m, 16 * sizeof(GLfloat));
}

void bridge_MultMatrixf(const GLfloat *m)
{
    GLfloat *cur = GetCurrentMatrix();
    Mat4Multiply(cur, m, cur);
}

void bridge_Translatef(GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat t[16];
    Mat4Identity(t);
    t[12] = x;
    t[13] = y;
    t[14] = z;
    bridge_MultMatrixf(t);
}

void bridge_Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat rad = angle * (float)M_PI / 180.0f;
    GLfloat c = cosf(rad);
    GLfloat s = sinf(rad);
    GLfloat len = sqrtf(x*x + y*y + z*z);
    if (len < 1e-6f) return;
    x /= len; y /= len; z /= len;

    GLfloat r[16];
    r[0] = x*x*(1-c)+c;   r[4] = x*y*(1-c)-z*s; r[8]  = x*z*(1-c)+y*s; r[12] = 0;
    r[1] = y*x*(1-c)+z*s; r[5] = y*y*(1-c)+c;   r[9]  = y*z*(1-c)-x*s; r[13] = 0;
    r[2] = x*z*(1-c)-y*s; r[6] = y*z*(1-c)+x*s; r[10] = z*z*(1-c)+c;   r[14] = 0;
    r[3] = 0;             r[7] = 0;              r[11] = 0;              r[15] = 1;
    bridge_MultMatrixf(r);
}

void bridge_Scalef(GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat s[16];
    Mat4Identity(s);
    s[0] = x;
    s[5] = y;
    s[10] = z;
    bridge_MultMatrixf(s);
}

void bridge_Orthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal)
{
    GLfloat m[16];
    Mat4Identity(m);
    m[0]  = 2.0f / (right - left);
    m[5]  = 2.0f / (top - bottom);
    m[10] = -2.0f / (farVal - nearVal);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(farVal + nearVal) / (farVal - nearVal);
    bridge_MultMatrixf(m);
}

void bridge_Frustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal)
{
    GLfloat m[16];
    memset(m, 0, sizeof(m));
    m[0]  = 2.0f * nearVal / (right - left);
    m[5]  = 2.0f * nearVal / (top - bottom);
    m[8]  = (right + left) / (right - left);
    m[9]  = (top + bottom) / (top - bottom);
    m[10] = -(farVal + nearVal) / (farVal - nearVal);
    m[11] = -1.0f;
    m[14] = -2.0f * farVal * nearVal / (farVal - nearVal);
    bridge_MultMatrixf(m);
}

// ============================================================================
// Shader state sync - called before every draw call
// ============================================================================

void bridge_SyncShaderState(void)
{
    glUseProgram(sShaderProgram);

    // Matrices
    glUniformMatrix4fv(sLoc_projection, 1, GL_FALSE, sProjectionStack.stack[sProjectionStack.top]);
    glUniformMatrix4fv(sLoc_modelview, 1, GL_FALSE, sModelviewStack.stack[sModelviewStack.top]);

    // Normal matrix
    GLfloat normalMatrix[9];
    ExtractNormalMatrix(sModelviewStack.stack[sModelviewStack.top], normalMatrix);
    glUniformMatrix3fv(sLoc_normalMatrix, 1, GL_FALSE, normalMatrix);

    // Lighting
    glUniform1i(sLoc_lightingEnabled, sLightingEnabled);
    glUniform4fv(sLoc_ambientLight, 1, sAmbientLight);
    glUniform1i(sLoc_light0Enabled, sLights[0].enabled);
    glUniform4fv(sLoc_light0Position, 1, sLights[0].position);
    glUniform4fv(sLoc_light0Diffuse, 1, sLights[0].diffuse);
    glUniform1i(sLoc_light1Enabled, sLights[1].enabled);
    glUniform4fv(sLoc_light1Position, 1, sLights[1].position);
    glUniform4fv(sLoc_light1Diffuse, 1, sLights[1].diffuse);
    glUniform1i(sLoc_light2Enabled, sLights[2].enabled);
    glUniform4fv(sLoc_light2Position, 1, sLights[2].position);
    glUniform4fv(sLoc_light2Diffuse, 1, sLights[2].diffuse);
    glUniform1i(sLoc_light3Enabled, sLights[3].enabled);
    glUniform4fv(sLoc_light3Position, 1, sLights[3].position);
    glUniform4fv(sLoc_light3Diffuse, 1, sLights[3].diffuse);

    // Fog
    glUniform1i(sLoc_fogEnabled, sFogEnabled);
    glUniform1f(sLoc_fogStart, sFogStart);
    glUniform1f(sLoc_fogEnd, sFogEnd);
    glUniform4fv(sLoc_fogColor, 1, sFogColor);

    // Texture
    glUniform1i(sLoc_textureEnabled, sTexture2DEnabled);
    glUniform1i(sLoc_texture0, 0);

    // Multi-texture & texgen
    glUniform1i(sLoc_texture1Enabled, sTexture1Enabled);
    glUniform1i(sLoc_texture1, 1);
    glUniform1i(sLoc_texEnvMode1, sTexEnvMode1);
    glUniform1i(sLoc_texGenSEnabled, sTexGenSEnabled);
    glUniform1i(sLoc_texGenTEnabled, sTexGenTEnabled);

    // Alpha test
    glUniform1i(sLoc_alphaTestEnabled, sAlphaTestEnabled);
    glUniform1f(sLoc_alphaRef, sAlphaTestRef);
    glUniform1i(sLoc_alphaFunc, (GLint)sAlphaTestFunc);
}

// ============================================================================
// Immediate mode
// ============================================================================

void bridge_Begin(GLenum mode)
{
    sImmMode = mode;
    sImmVertexCount = 0;
    sInImmMode = GL_TRUE;
}

static void ImmAddVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    if (sImmVertexCount >= BRIDGE_IMM_MAX_VERTICES) {
        BRIDGE_LOGE("Immediate mode vertex overflow!");
        return;
    }
    int base = sImmVertexCount * IMM_FLOATS_PER_VERTEX;
    // Position
    sImmVertices[base + 0] = x;
    sImmVertices[base + 1] = y;
    sImmVertices[base + 2] = z;
    // Normal
    sImmVertices[base + 3] = sCurrentNormal[0];
    sImmVertices[base + 4] = sCurrentNormal[1];
    sImmVertices[base + 5] = sCurrentNormal[2];
    // TexCoord
    sImmVertices[base + 6] = sCurrentTexCoord[0];
    sImmVertices[base + 7] = sCurrentTexCoord[1];
    // Color
    sImmVertices[base + 8]  = sCurrentColor[0];
    sImmVertices[base + 9]  = sCurrentColor[1];
    sImmVertices[base + 10] = sCurrentColor[2];
    sImmVertices[base + 11] = sCurrentColor[3];
    sImmVertexCount++;
    (void)w;
}

void bridge_End(void)
{
    if (!sInImmMode || sImmVertexCount == 0) {
        sInImmMode = GL_FALSE;
        return;
    }
    sInImmMode = GL_FALSE;

    bridge_SyncShaderState();

    glBindVertexArray(sVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sImmVBO);
    glBufferData(GL_ARRAY_BUFFER, sImmVertexCount * IMM_FLOATS_PER_VERTEX * sizeof(GLfloat),
                 sImmVertices, GL_DYNAMIC_DRAW);

    // Position
    glEnableVertexAttribArray(ATTR_POSITION);
    glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE,
                          IMM_FLOATS_PER_VERTEX * sizeof(GLfloat), (void*)0);
    // Normal
    glEnableVertexAttribArray(ATTR_NORMAL);
    glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE,
                          IMM_FLOATS_PER_VERTEX * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    // TexCoord
    glEnableVertexAttribArray(ATTR_TEXCOORD);
    glVertexAttribPointer(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                          IMM_FLOATS_PER_VERTEX * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    // Color
    glEnableVertexAttribArray(ATTR_COLOR);
    glVertexAttribPointer(ATTR_COLOR, 4, GL_FLOAT, GL_FALSE,
                          IMM_FLOATS_PER_VERTEX * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));

    // TexCoord1 - immediate mode doesn't use multi-texture, set default
    glDisableVertexAttribArray(ATTR_TEXCOORD1);
    {
        GLfloat defaultTC1[2] = {0, 0};
        glVertexAttrib2fv(ATTR_TEXCOORD1, defaultTC1);
    }

    // Convert quads to triangles
    if (sImmMode == GL_QUADS) {
        // Every 4 vertices forms a quad -> 2 triangles
        int numQuads = sImmVertexCount / 4;
        int numIndices = numQuads * 6;
        GLushort *indices = (GLushort*)malloc(numIndices * sizeof(GLushort));
        if (indices) {
            for (int q = 0; q < numQuads; q++) {
                int base = q * 4;
                indices[q*6+0] = base + 0;
                indices[q*6+1] = base + 1;
                indices[q*6+2] = base + 2;
                indices[q*6+3] = base + 0;
                indices[q*6+4] = base + 2;
                indices[q*6+5] = base + 3;
            }
            glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);
            free(indices);
        }
    }
    else if (sImmMode == GL_QUAD_STRIP) {
        // Quad strip: every 2 new vertices form a quad with the previous 2
        int numQuads = (sImmVertexCount - 2) / 2;
        int numIndices = numQuads * 6;
        GLushort *indices = (GLushort*)malloc(numIndices * sizeof(GLushort));
        if (indices) {
            for (int q = 0; q < numQuads; q++) {
                int base = q * 2;
                indices[q*6+0] = base + 0;
                indices[q*6+1] = base + 1;
                indices[q*6+2] = base + 3;
                indices[q*6+3] = base + 0;
                indices[q*6+4] = base + 3;
                indices[q*6+5] = base + 2;
            }
            glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);
            free(indices);
        }
    }
    else if (sImmMode == GL_POLYGON) {
        // Polygon: treat as triangle fan
        glDrawArrays(GL_TRIANGLE_FAN, 0, sImmVertexCount);
    }
    else {
        // GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_POINTS
        glDrawArrays(sImmMode, 0, sImmVertexCount);
    }

    glDisableVertexAttribArray(ATTR_POSITION);
    glDisableVertexAttribArray(ATTR_NORMAL);
    glDisableVertexAttribArray(ATTR_TEXCOORD);
    glDisableVertexAttribArray(ATTR_COLOR);
    glDisableVertexAttribArray(ATTR_TEXCOORD1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void bridge_Vertex2f(GLfloat x, GLfloat y)
{
    ImmAddVertex(x, y, 0.0f, 1.0f);
}

void bridge_Vertex2i(GLint x, GLint y)
{
    ImmAddVertex((GLfloat)x, (GLfloat)y, 0.0f, 1.0f);
}

void bridge_Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    ImmAddVertex(x, y, z, 1.0f);
}

void bridge_Vertex3fv(const GLfloat *v)
{
    ImmAddVertex(v[0], v[1], v[2], 1.0f);
}

void bridge_Vertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    ImmAddVertex((GLfloat)x, (GLfloat)y, (GLfloat)z, 1.0f);
}

void bridge_Vertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    ImmAddVertex(x, y, z, w);
}

void bridge_TexCoord2f(GLfloat s, GLfloat t)
{
    sCurrentTexCoord[0] = s;
    sCurrentTexCoord[1] = t;
}

void bridge_TexCoord2d(GLdouble s, GLdouble t)
{
    sCurrentTexCoord[0] = (GLfloat)s;
    sCurrentTexCoord[1] = (GLfloat)t;
}

void bridge_TexCoord2i(GLint s, GLint t)
{
    sCurrentTexCoord[0] = (GLfloat)s;
    sCurrentTexCoord[1] = (GLfloat)t;
}

void bridge_TexCoord2fv(const GLfloat *v)
{
    sCurrentTexCoord[0] = v[0];
    sCurrentTexCoord[1] = v[1];
}

void bridge_Normal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    sCurrentNormal[0] = nx;
    sCurrentNormal[1] = ny;
    sCurrentNormal[2] = nz;
}

void bridge_Normal3fv(const GLfloat *v)
{
    sCurrentNormal[0] = v[0];
    sCurrentNormal[1] = v[1];
    sCurrentNormal[2] = v[2];
}

void bridge_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    sCurrentColor[0] = r;
    sCurrentColor[1] = g;
    sCurrentColor[2] = b;
    sCurrentColor[3] = a;
}

void bridge_Color3f(GLfloat r, GLfloat g, GLfloat b)
{
    sCurrentColor[0] = r;
    sCurrentColor[1] = g;
    sCurrentColor[2] = b;
    sCurrentColor[3] = 1.0f;
}

void bridge_Color4fv(const GLfloat *v)
{
    sCurrentColor[0] = v[0];
    sCurrentColor[1] = v[1];
    sCurrentColor[2] = v[2];
    sCurrentColor[3] = v[3];
}

void bridge_Color3fv(const GLfloat *v)
{
    sCurrentColor[0] = v[0];
    sCurrentColor[1] = v[1];
    sCurrentColor[2] = v[2];
    sCurrentColor[3] = 1.0f;
}

// ============================================================================
// Enable/Disable emulation
// ============================================================================

void bridge_Enable(GLenum cap)
{
    switch (cap) {
        case GL_LIGHTING:     sLightingEnabled = GL_TRUE; break;
        case GL_LIGHT0:       sLights[0].enabled = GL_TRUE; break;
        case GL_LIGHT1:       sLights[1].enabled = GL_TRUE; break;
        case GL_LIGHT2:       sLights[2].enabled = GL_TRUE; break;
        case GL_LIGHT3:       sLights[3].enabled = GL_TRUE; break;
        case GL_FOG:          sFogEnabled = GL_TRUE; break;
        case GL_TEXTURE_2D:
            if (sActiveTexture == GL_TEXTURE1)
                sTexture1Enabled = GL_TRUE;
            else
                sTexture2DEnabled = GL_TRUE;
            break;
        case GL_NORMALIZE:    sNormalizeEnabled = GL_TRUE; break;
        case GL_RESCALE_NORMAL: break; // no-op
        case GL_ALPHA_TEST:   sAlphaTestEnabled = GL_TRUE; break;
        case GL_COLOR_MATERIAL: break; // always on in our model
        case GL_TEXTURE_GEN_S: sTexGenSEnabled = GL_TRUE; break;
        case GL_TEXTURE_GEN_T: sTexGenTEnabled = GL_TRUE; break;
        // Native GLES 3.0 caps
        case GL_DEPTH_TEST:
        case GL_CULL_FACE:
        case GL_BLEND:
        case GL_SCISSOR_TEST:
        case GL_STENCIL_TEST:
        case GL_DITHER:
        case GL_POLYGON_OFFSET_FILL:
        case GL_SAMPLE_ALPHA_TO_COVERAGE:
        case GL_SAMPLE_COVERAGE:
            glEnable(cap);
            break;
        default:
            // Unknown cap, try to pass through
            break;
    }
}

void bridge_Disable(GLenum cap)
{
    switch (cap) {
        case GL_LIGHTING:     sLightingEnabled = GL_FALSE; break;
        case GL_LIGHT0:       sLights[0].enabled = GL_FALSE; break;
        case GL_LIGHT1:       sLights[1].enabled = GL_FALSE; break;
        case GL_LIGHT2:       sLights[2].enabled = GL_FALSE; break;
        case GL_LIGHT3:       sLights[3].enabled = GL_FALSE; break;
        case GL_FOG:          sFogEnabled = GL_FALSE; break;
        case GL_TEXTURE_2D:
            if (sActiveTexture == GL_TEXTURE1)
                sTexture1Enabled = GL_FALSE;
            else
                sTexture2DEnabled = GL_FALSE;
            break;
        case GL_NORMALIZE:    sNormalizeEnabled = GL_FALSE; break;
        case GL_RESCALE_NORMAL: break;
        case GL_ALPHA_TEST:   sAlphaTestEnabled = GL_FALSE; break;
        case GL_COLOR_MATERIAL: break;
        case GL_TEXTURE_GEN_S: sTexGenSEnabled = GL_FALSE; break;
        case GL_TEXTURE_GEN_T: sTexGenTEnabled = GL_FALSE; break;
        case GL_DEPTH_TEST:
        case GL_CULL_FACE:
        case GL_BLEND:
        case GL_SCISSOR_TEST:
        case GL_STENCIL_TEST:
        case GL_DITHER:
        case GL_POLYGON_OFFSET_FILL:
        case GL_SAMPLE_ALPHA_TO_COVERAGE:
        case GL_SAMPLE_COVERAGE:
            glDisable(cap);
            break;
        default:
            break;
    }
}

GLboolean bridge_IsEnabled(GLenum cap)
{
    switch (cap) {
        case GL_LIGHTING:     return sLightingEnabled;
        case GL_LIGHT0:       return sLights[0].enabled;
        case GL_LIGHT1:       return sLights[1].enabled;
        case GL_LIGHT2:       return sLights[2].enabled;
        case GL_LIGHT3:       return sLights[3].enabled;
        case GL_FOG:          return sFogEnabled;
        case GL_TEXTURE_2D:   return sTexture2DEnabled;
        case GL_NORMALIZE:    return sNormalizeEnabled;
        case GL_ALPHA_TEST:   return sAlphaTestEnabled;
        case GL_COLOR_MATERIAL: return GL_TRUE;
        case GL_TEXTURE_GEN_S: return sTexGenSEnabled;
        case GL_TEXTURE_GEN_T: return sTexGenTEnabled;
        case GL_DEPTH_TEST:
        case GL_CULL_FACE:
        case GL_BLEND:
        case GL_SCISSOR_TEST:
        case GL_STENCIL_TEST:
        case GL_DITHER:
            return glIsEnabled(cap);
        default:
            return GL_FALSE;
    }
}

// ============================================================================
// Lighting
// ============================================================================

void bridge_Lightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    int idx = light - GL_LIGHT0;
    if (idx < 0 || idx >= BRIDGE_MAX_LIGHTS) return;

    switch (pname) {
        case GL_POSITION:
            // Transform light position by current modelview matrix
            {
                const GLfloat *mv = sModelviewStack.stack[sModelviewStack.top];
                GLfloat x = params[0], y = params[1], z = params[2], w = params[3];
                sLights[idx].position[0] = mv[0]*x + mv[4]*y + mv[8]*z + mv[12]*w;
                sLights[idx].position[1] = mv[1]*x + mv[5]*y + mv[9]*z + mv[13]*w;
                sLights[idx].position[2] = mv[2]*x + mv[6]*y + mv[10]*z + mv[14]*w;
                sLights[idx].position[3] = mv[3]*x + mv[7]*y + mv[11]*z + mv[15]*w;
            }
            break;
        case GL_AMBIENT:
            memcpy(sLights[idx].ambient, params, 4 * sizeof(GLfloat));
            break;
        case GL_DIFFUSE:
            memcpy(sLights[idx].diffuse, params, 4 * sizeof(GLfloat));
            break;
        case GL_SPECULAR:
            // Ignore specular for now
            break;
        default:
            break;
    }
}

void bridge_LightModelfv(GLenum pname, const GLfloat *params)
{
    if (pname == GL_LIGHT_MODEL_AMBIENT) {
        memcpy(sAmbientLight, params, 4 * sizeof(GLfloat));
    }
}

void bridge_LightModelf(GLenum pname, GLfloat param)
{
    (void)pname; (void)param;
}

void bridge_LightModeli(GLenum pname, GLint param)
{
    (void)pname; (void)param;
}

void bridge_Materialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    (void)face;
    // In our simplified model, material color is the current vertex color
    // We track this for color material mode
    if (pname == GL_AMBIENT_AND_DIFFUSE || pname == GL_DIFFUSE) {
        sCurrentColor[0] = params[0];
        sCurrentColor[1] = params[1];
        sCurrentColor[2] = params[2];
        sCurrentColor[3] = params[3];
    }
}

// ============================================================================
// Fog
// ============================================================================

void bridge_Fogf(GLenum pname, GLfloat param)
{
    switch (pname) {
        case GL_FOG_START:   sFogStart = param; break;
        case GL_FOG_END:     sFogEnd = param; break;
        case GL_FOG_DENSITY: break; // We use linear fog
        case GL_FOG_MODE:    sFogMode = (GLint)param; break;
        default: break;
    }
}

void bridge_Fogfv(GLenum pname, const GLfloat *params)
{
    if (pname == GL_FOG_COLOR) {
        memcpy(sFogColor, params, 4 * sizeof(GLfloat));
    } else {
        bridge_Fogf(pname, params[0]);
    }
}

void bridge_Fogi(GLenum pname, GLint param)
{
    bridge_Fogf(pname, (GLfloat)param);
}

// ============================================================================
// Texture environment (simplified)
// ============================================================================

void bridge_TexEnvi(GLenum target, GLenum pname, GLint param)
{
    (void)target;
    // Track texture environment mode for multi-texture combining.
    // We only update unit 1's mode when unit 1 is the active texture,
    // which is the case when the game calls this after glActiveTextureARB(GL_TEXTURE1).
    if (sActiveTexture != GL_TEXTURE1) {
        return;  // Only track for texture unit 1; unit 0 always uses modulate
    }
    if (pname == GL_TEXTURE_ENV_MODE) {
        if (param == GL_MODULATE) {
            sTexEnvMode1 = 0;
        } else if (param == GL_ADD) {
            sTexEnvMode1 = 1;  // ADD
        } else if (param == GL_COMBINE || param == GL_COMBINE_EXT) {
            // GL_COMBINE uses additional parameters (GL_COMBINE_RGB) to set the actual operation.
            // The game only uses GL_COMBINE + GL_ADD via GL_COMBINE_RGB, which we handle below.
            // Default to modulate until GL_COMBINE_RGB is set.
            sTexEnvMode1 = 0;
        } else if (param == GL_REPLACE) {
            sTexEnvMode1 = 2;
        }
    } else if (pname == GL_COMBINE_RGB || pname == GL_COMBINE_RGB_EXT) {
        // This is called after GL_TEXTURE_ENV_MODE = GL_COMBINE to specify the RGB combine operation
        if (param == GL_ADD) {
            sTexEnvMode1 = 1;
        }
    }
}

// ============================================================================
// Alpha test
// ============================================================================

void bridge_AlphaFunc(GLenum func, GLfloat ref)
{
    sAlphaTestFunc = func;
    sAlphaTestRef = ref;
}

// ============================================================================
// Vertex arrays
// ============================================================================

void bridge_EnableClientState(GLenum cap)
{
    switch (cap) {
        case GL_VERTEX_ARRAY:        sVertexArrayEnabled = GL_TRUE; break;
        case GL_NORMAL_ARRAY:        sNormalArrayEnabled = GL_TRUE; break;
        case GL_COLOR_ARRAY:         sColorArrayEnabled = GL_TRUE; break;
        case GL_TEXTURE_COORD_ARRAY:
            if (sClientActiveTexture == GL_TEXTURE1)
                sTexCoord1ArrayEnabled = GL_TRUE;
            else
                sTexCoordArrayEnabled = GL_TRUE;
            break;
        default: break;
    }
}

void bridge_DisableClientState(GLenum cap)
{
    switch (cap) {
        case GL_VERTEX_ARRAY:        sVertexArrayEnabled = GL_FALSE; break;
        case GL_NORMAL_ARRAY:        sNormalArrayEnabled = GL_FALSE; break;
        case GL_COLOR_ARRAY:         sColorArrayEnabled = GL_FALSE; break;
        case GL_TEXTURE_COORD_ARRAY:
            if (sClientActiveTexture == GL_TEXTURE1)
                sTexCoord1ArrayEnabled = GL_FALSE;
            else
                sTexCoordArrayEnabled = GL_FALSE;
            break;
        default: break;
    }
}

void bridge_VertexPointer(GLint size, GLenum type, GLsizei stride, const void *ptr)
{
    sVertexArraySize = size;
    sVertexArrayType = type;
    sVertexArrayStride = stride;
    sVertexArrayPtr = ptr;
}

void bridge_NormalPointer(GLenum type, GLsizei stride, const void *ptr)
{
    sNormalArrayType = type;
    sNormalArrayStride = stride;
    sNormalArrayPtr = ptr;
}

void bridge_ColorPointer(GLint size, GLenum type, GLsizei stride, const void *ptr)
{
    sColorArraySize = size;
    sColorArrayType = type;
    sColorArrayStride = stride;
    sColorArrayPtr = ptr;
}

void bridge_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *ptr)
{
    if (sClientActiveTexture == GL_TEXTURE1) {
        sTexCoord1ArraySize = size;
        sTexCoord1ArrayType = type;
        sTexCoord1ArrayStride = stride;
        sTexCoord1ArrayPtr = ptr;
    } else {
        sTexCoordArraySize = size;
        sTexCoordArrayType = type;
        sTexCoordArrayStride = stride;
        sTexCoordArrayPtr = ptr;
    }
}

void bridge_ClientActiveTexture(GLenum texture)
{
    sClientActiveTexture = texture;
}

void bridge_ActiveTexture(GLenum texture)
{
    sActiveTexture = texture;
    glActiveTexture(texture);
}

// ============================================================================
// Vertex array draw setup
// ============================================================================

static void SetupVertexAttribsForArrayDraw(void)
{
    bridge_SyncShaderState();

    glBindVertexArray(sVAO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // We're using client-side arrays

    // Position
    if (sVertexArrayEnabled && sVertexArrayPtr) {
        glEnableVertexAttribArray(ATTR_POSITION);
        glVertexAttribPointer(ATTR_POSITION, sVertexArraySize, sVertexArrayType,
                              GL_FALSE, sVertexArrayStride, sVertexArrayPtr);
    } else {
        glDisableVertexAttribArray(ATTR_POSITION);
    }

    // Normal
    if (sNormalArrayEnabled && sNormalArrayPtr) {
        glEnableVertexAttribArray(ATTR_NORMAL);
        glVertexAttribPointer(ATTR_NORMAL, 3, sNormalArrayType,
                              GL_FALSE, sNormalArrayStride, sNormalArrayPtr);
    } else {
        glDisableVertexAttribArray(ATTR_NORMAL);
        glVertexAttrib3fv(ATTR_NORMAL, sCurrentNormal);
    }

    // TexCoord
    if (sTexCoordArrayEnabled && sTexCoordArrayPtr) {
        glEnableVertexAttribArray(ATTR_TEXCOORD);
        glVertexAttribPointer(ATTR_TEXCOORD, sTexCoordArraySize, sTexCoordArrayType,
                              GL_FALSE, sTexCoordArrayStride, sTexCoordArrayPtr);
    } else {
        glDisableVertexAttribArray(ATTR_TEXCOORD);
        glVertexAttrib2fv(ATTR_TEXCOORD, sCurrentTexCoord);
    }

    // Color
    if (sColorArrayEnabled && sColorArrayPtr) {
        glEnableVertexAttribArray(ATTR_COLOR);
        GLboolean normalize = (sColorArrayType == GL_UNSIGNED_BYTE) ? GL_TRUE : GL_FALSE;
        glVertexAttribPointer(ATTR_COLOR, sColorArraySize, sColorArrayType,
                              normalize, sColorArrayStride, sColorArrayPtr);
    } else {
        glDisableVertexAttribArray(ATTR_COLOR);
        glVertexAttrib4fv(ATTR_COLOR, sCurrentColor);
    }

    // TexCoord1 (multi-texture unit 1)
    if (sTexCoord1ArrayEnabled && sTexCoord1ArrayPtr) {
        glEnableVertexAttribArray(ATTR_TEXCOORD1);
        glVertexAttribPointer(ATTR_TEXCOORD1, sTexCoord1ArraySize, sTexCoord1ArrayType,
                              GL_FALSE, sTexCoord1ArrayStride, sTexCoord1ArrayPtr);
    } else {
        glDisableVertexAttribArray(ATTR_TEXCOORD1);
        GLfloat defaultTC1[2] = {0, 0};
        glVertexAttrib2fv(ATTR_TEXCOORD1, defaultTC1);
    }
}

static void CleanupVertexAttribs(void)
{
    glDisableVertexAttribArray(ATTR_POSITION);
    glDisableVertexAttribArray(ATTR_NORMAL);
    glDisableVertexAttribArray(ATTR_TEXCOORD);
    glDisableVertexAttribArray(ATTR_COLOR);
    glDisableVertexAttribArray(ATTR_TEXCOORD1);
    glBindVertexArray(0);
}

// ============================================================================
// Draw calls
// ============================================================================

void bridge_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    SetupVertexAttribsForArrayDraw();
    glDrawArrays(mode, first, count);
    CleanupVertexAttribs();
}

void bridge_DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    SetupVertexAttribsForArrayDraw();
    glDrawElements(mode, count, type, indices);
    CleanupVertexAttribs();
}

// ============================================================================
// State queries
// ============================================================================

void bridge_GetFloatv(GLenum pname, GLfloat *params)
{
    switch (pname) {
        case GL_MODELVIEW_MATRIX:
            memcpy(params, sModelviewStack.stack[sModelviewStack.top], 16 * sizeof(GLfloat));
            break;
        case GL_PROJECTION_MATRIX:
            memcpy(params, sProjectionStack.stack[sProjectionStack.top], 16 * sizeof(GLfloat));
            break;
        case GL_CURRENT_COLOR:
            memcpy(params, sCurrentColor, 4 * sizeof(GLfloat));
            break;
        default:
            glGetFloatv(pname, params);
            break;
    }
}

void bridge_GetIntegerv(GLenum pname, GLint *params)
{
    switch (pname) {
        case GL_BLEND_SRC:
            *params = sBlendSrc;
            break;
        case GL_BLEND_DST:
            *params = sBlendDst;
            break;
        default:
            glGetIntegerv(pname, params);
            break;
    }
}

void bridge_GetBooleanv(GLenum pname, GLboolean *params)
{
    switch (pname) {
        case GL_DEPTH_WRITEMASK:
            *params = sDepthMask;
            break;
        default:
            glGetBooleanv(pname, params);
            break;
    }
}

// ============================================================================
// Blend function tracking
// ============================================================================

void bridge_BlendFunc(GLenum sfactor, GLenum dfactor)
{
    sBlendSrc = sfactor;
    sBlendDst = dfactor;
    glBlendFunc(sfactor, dfactor);
}

// ============================================================================
// Depth mask tracking
// ============================================================================

void bridge_DepthMask(GLboolean flag)
{
    sDepthMask = flag;
    glDepthMask(flag);
}

// ============================================================================
// Misc stubs / simple implementations
// ============================================================================

void bridge_ColorMaterial(GLenum face, GLenum mode)
{
    (void)face; (void)mode;
    // Always in color material mode in our simplified model
}

void bridge_PolygonMode(GLenum face, GLenum mode)
{
    (void)face; (void)mode;
    // Not supported in GLES 3.0 - no wireframe mode
}

void bridge_TexGeni(GLenum coord, GLenum pname, GLint param)
{
    (void)coord; (void)pname; (void)param;
    // The game only uses sphere map mode (GL_SPHERE_MAP) for texgen.
    // The sphere map computation is always active when texgen S/T are enabled.
}

void bridge_TexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    (void)coord; (void)pname; (void)param;
}

void bridge_TexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    (void)coord; (void)pname; (void)params;
}

void bridge_Hint(GLenum target, GLenum mode)
{
    // Only pass through hints that GLES 3.0 supports
    if (target == GL_GENERATE_MIPMAP_HINT || target == GL_FRAGMENT_SHADER_DERIVATIVE_HINT) {
        glHint(target, mode);
    }
    // Silently ignore GL_FOG_HINT, GL_LINE_SMOOTH_HINT, etc.
}

#endif // __ANDROID__
