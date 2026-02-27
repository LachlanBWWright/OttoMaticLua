//
// modern_gl.c
// Modern OpenGL/WebGL implementation for Emscripten
//

#ifdef __EMSCRIPTEN__

#include "game.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GLES2/gl2.h>

/****************************/
/*    GLOBALS               */
/****************************/

ModernGLShaderProgram gModernGLShader;
ModernGLState gModernGLState;
ImmediateModeBuffer gImmediateModeBuffer;

/****************************/
/*    SHADER SOURCE         */
/****************************/

// Vertex shader source (embedded)
static const char* gVertexShaderSource =
"precision highp float;\n"
"attribute vec3 aPosition;\n"
"attribute vec3 aNormal;\n"
"attribute vec4 aColor;\n"
"attribute vec2 aTexCoord0;\n"
"attribute vec2 aTexCoord1;\n"
"uniform mat4 uMVPMatrix;\n"
"uniform mat4 uModelViewMatrix;\n"
"uniform mediump mat3 uNormalMatrix;\n"
"uniform vec3 uAmbientLight;\n"
"uniform vec3 uLightDirection[4];\n"
"uniform vec3 uLightColor[4];\n"
"uniform int uNumLights;\n"
"uniform bool uFogEnabled;\n"
"uniform float uFogStart;\n"
"uniform float uFogEnd;\n"
"uniform float uFogDensity;\n"
"uniform int uFogMode;\n"
"uniform vec4 uMaterialColor;\n"
"uniform bool uUseLighting;\n"
"uniform bool uUseVertexColor;\n"
"uniform bool uUseTexture0;\n"
"uniform bool uUseTexture1;\n"
"uniform mat4 uTextureMatrix;\n"
"varying vec4 vColor;\n"
"varying vec2 vTexCoord0;\n"
"varying vec2 vTexCoord1;\n"
"varying float vFogFactor;\n"
"varying vec3 vNormal;\n"
"void main() {\n"
"    gl_Position = uMVPMatrix * vec4(aPosition, 1.0);\n"
"    vec4 viewPos = uModelViewMatrix * vec4(aPosition, 1.0);\n"
"    float fogCoord = length(viewPos.xyz);\n"
"    if (uFogEnabled) {\n"
"        if (uFogMode == 0)\n"
"            vFogFactor = (uFogEnd - fogCoord) / (uFogEnd - uFogStart);\n"
"        else if (uFogMode == 1)\n"
"            vFogFactor = exp(-uFogDensity * fogCoord);\n"
"        else\n"
"            vFogFactor = exp(-pow(uFogDensity * fogCoord, 2.0));\n"
"        vFogFactor = clamp(vFogFactor, 0.0, 1.0);\n"
"    } else {\n"
"        vFogFactor = 1.0;\n"
"    }\n"
"    vec3 normal = normalize(uNormalMatrix * aNormal);\n"
"    vNormal = normal;\n"
"    vec4 finalColor = uMaterialColor;\n"
"    if (uUseLighting) {\n"
"        vec3 ambient = uAmbientLight;\n"
"        vec3 diffuse = vec3(0.0);\n"
"        for (int i = 0; i < 4; i++) {\n"
"            if (i >= uNumLights) break;\n"
"            float NdotL = max(dot(normal, uLightDirection[i]), 0.0);\n"
"            diffuse += uLightColor[i] * NdotL;\n"
"        }\n"
"        finalColor.rgb *= (ambient + diffuse);\n"
"    }\n"
"    if (uUseVertexColor) finalColor *= aColor;\n"
"    vColor = finalColor;\n"
"    if (uUseTexture0) {\n"
"        vec4 texCoord = uTextureMatrix * vec4(aTexCoord0, 0.0, 1.0);\n"
"        vTexCoord0 = texCoord.xy;\n"
"    } else {\n"
"        vTexCoord0 = aTexCoord0;\n"
"    }\n"
"    vTexCoord1 = aTexCoord1;\n"
"}\n";

// Fragment shader source (embedded)
static const char* gFragmentShaderSource =
"precision mediump float;\n"
"uniform sampler2D uTexture0;\n"
"uniform sampler2D uTexture1;\n"
"uniform bool uUseTexture0;\n"
"uniform bool uUseTexture1;\n"
"uniform int uMultiTextureMode;\n"
"uniform int uMultiTextureCombine;\n"
"uniform bool uUseSphereMap;\n"
"uniform mediump mat3 uNormalMatrix;\n"
"uniform bool uFogEnabled;\n"
"uniform vec3 uFogColor;\n"
"uniform bool uAlphaTestEnabled;\n"
"uniform int uAlphaFunc;\n"
"uniform float uAlphaRef;\n"
"uniform float uGlobalTransparency;\n"
"uniform vec3 uGlobalColorFilter;\n"
"varying vec4 vColor;\n"
"varying vec2 vTexCoord0;\n"
"varying vec2 vTexCoord1;\n"
"varying float vFogFactor;\n"
"varying vec3 vNormal;\n"
"void main() {\n"
"    vec4 color = vColor;\n"
"    if (uUseTexture0) {\n"
"        vec4 texColor = texture2D(uTexture0, vTexCoord0);\n"
"        color *= texColor;\n"
"    }\n"
"    if (uUseTexture1) {\n"
"        vec2 texCoord1 = vTexCoord1;\n"
"        if (uUseSphereMap) {\n"
"            vec3 normal = normalize(vNormal);\n"
"            vec3 viewNormal = normalize(uNormalMatrix * normal);\n"
"            texCoord1 = viewNormal.xy * 0.5 + 0.5;\n"
"        }\n"
"        vec4 texColor1 = texture2D(uTexture1, texCoord1);\n"
"        if (uMultiTextureCombine == 0)\n"
"            color *= texColor1;\n"
"        else if (uMultiTextureCombine == 1)\n"
"            color.rgb += texColor1.rgb;\n"
"    }\n"
"    color.rgb *= uGlobalColorFilter;\n"
"    color.a *= uGlobalTransparency;\n"
"    if (uAlphaTestEnabled) {\n"
"        bool discard_frag = false;\n"
"        if (uAlphaFunc == 0) discard_frag = true;\n"
"        else if (uAlphaFunc == 1) discard_frag = color.a >= uAlphaRef;\n"
"        else if (uAlphaFunc == 2) discard_frag = abs(color.a - uAlphaRef) > 0.001;\n"
"        else if (uAlphaFunc == 3) discard_frag = color.a > uAlphaRef;\n"
"        else if (uAlphaFunc == 4) discard_frag = color.a <= uAlphaRef;\n"
"        else if (uAlphaFunc == 5) discard_frag = abs(color.a - uAlphaRef) < 0.001;\n"
"        else if (uAlphaFunc == 6) discard_frag = color.a < uAlphaRef;\n"
"        if (discard_frag) discard;\n"
"    }\n"
"    if (uFogEnabled) {\n"
"        color.rgb = mix(uFogColor, color.rgb, vFogFactor);\n"
"    }\n"
"    gl_FragColor = color;\n"
"}\n";

/****************************/
/*    HELPER FUNCTIONS      */
/****************************/

static GLuint CompileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        const char* shaderType = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        printf("[ModernGL] %s shader compilation failed:\n%s\n", shaderType, infoLog);
        return 0;
    }

    return shader;
}

static GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // Bind attribute locations before linking
    glBindAttribLocation(program, ATTRIB_LOCATION_POSITION, "aPosition");
    glBindAttribLocation(program, ATTRIB_LOCATION_NORMAL, "aNormal");
    glBindAttribLocation(program, ATTRIB_LOCATION_COLOR, "aColor");
    glBindAttribLocation(program, ATTRIB_LOCATION_TEXCOORD0, "aTexCoord0");
    glBindAttribLocation(program, ATTRIB_LOCATION_TEXCOORD1, "aTexCoord1");

    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        printf("[ModernGL] Program linking failed:\n%s\n", infoLog);

        // Also validate the program to get more detailed error information
        glValidateProgram(program);
        GLint validateStatus;
        glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);
        if (!validateStatus)
        {
            char validateLog[1024];
            glGetProgramInfoLog(program, 1024, NULL, validateLog);
            printf("[ModernGL] Program validation failed:\n%s\n", validateLog);
        }

        return 0;
    }

    return program;
}

/****************************/
/*    PUBLIC FUNCTIONS      */
/****************************/

void ModernGL_Init(void)
{
    printf("[ModernGL] Initializing modern GL subsystem...\n");

    // Initialize state
    memset(&gModernGLState, 0, sizeof(ModernGLState));
    gModernGLState.materialColor[0] = 1.0f;
    gModernGLState.materialColor[1] = 1.0f;
    gModernGLState.materialColor[2] = 1.0f;
    gModernGLState.materialColor[3] = 1.0f;
    gModernGLState.ambientLight[0] = 0.3f;
    gModernGLState.ambientLight[1] = 0.3f;
    gModernGLState.ambientLight[2] = 0.3f;
    gModernGLState.globalTransparency = 1.0f;
    gModernGLState.globalColorFilter[0] = 1.0f;
    gModernGLState.globalColorFilter[1] = 1.0f;
    gModernGLState.globalColorFilter[2] = 1.0f;
    gModernGLState.alphaRef = 0.0f;

    // Initialize texture matrix to identity
    for (int i = 0; i < 16; i++)
        gModernGLState.textureMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;

    // Initialize immediate mode buffer
    gImmediateModeBuffer.capacity = 1024;
    gImmediateModeBuffer.positions = (GLfloat*)malloc(gImmediateModeBuffer.capacity * 3 * sizeof(GLfloat));
    gImmediateModeBuffer.normals = (GLfloat*)malloc(gImmediateModeBuffer.capacity * 3 * sizeof(GLfloat));
    gImmediateModeBuffer.colors = (GLfloat*)malloc(gImmediateModeBuffer.capacity * 4 * sizeof(GLfloat));
    gImmediateModeBuffer.texCoords = (GLfloat*)malloc(gImmediateModeBuffer.capacity * 2 * sizeof(GLfloat));
    gImmediateModeBuffer.vertexCount = 0;
    gImmediateModeBuffer.currentColor[0] = 1.0f;
    gImmediateModeBuffer.currentColor[1] = 1.0f;
    gImmediateModeBuffer.currentColor[2] = 1.0f;
    gImmediateModeBuffer.currentColor[3] = 1.0f;
    gImmediateModeBuffer.currentNormal[2] = 1.0f;

    // Load shaders
    if (!ModernGL_LoadShaders())
    {
        printf("[ModernGL] ERROR: Failed to load shaders!\n");
    }

    printf("[ModernGL] Initialization complete\n");
}

void ModernGL_Shutdown(void)
{
    if (gModernGLShader.program)
    {
        glDeleteProgram(gModernGLShader.program);
    }

    if (gImmediateModeBuffer.positions)
    {
        free(gImmediateModeBuffer.positions);
        free(gImmediateModeBuffer.normals);
        free(gImmediateModeBuffer.colors);
        free(gImmediateModeBuffer.texCoords);
    }
}

Boolean ModernGL_LoadShaders(void)
{
    printf("[ModernGL] Loading shaders...\n");

    GLuint vertShader = CompileShader(GL_VERTEX_SHADER, gVertexShaderSource);
    if (!vertShader) return false;

    GLuint fragShader = CompileShader(GL_FRAGMENT_SHADER, gFragmentShaderSource);
    if (!fragShader)
    {
        glDeleteShader(vertShader);
        return false;
    }

    gModernGLShader.program = LinkProgram(vertShader, fragShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    if (!gModernGLShader.program) return false;

    // Get attribute locations
    gModernGLShader.aPosition = ATTRIB_LOCATION_POSITION;
    gModernGLShader.aNormal = ATTRIB_LOCATION_NORMAL;
    gModernGLShader.aColor = ATTRIB_LOCATION_COLOR;
    gModernGLShader.aTexCoord0 = ATTRIB_LOCATION_TEXCOORD0;
    gModernGLShader.aTexCoord1 = ATTRIB_LOCATION_TEXCOORD1;

    // Get uniform locations
    gModernGLShader.uMVPMatrix = glGetUniformLocation(gModernGLShader.program, "uMVPMatrix");
    gModernGLShader.uModelViewMatrix = glGetUniformLocation(gModernGLShader.program, "uModelViewMatrix");
    gModernGLShader.uNormalMatrix = glGetUniformLocation(gModernGLShader.program, "uNormalMatrix");
    gModernGLShader.uAmbientLight = glGetUniformLocation(gModernGLShader.program, "uAmbientLight");
    gModernGLShader.uLightDirection = glGetUniformLocation(gModernGLShader.program, "uLightDirection");
    gModernGLShader.uLightColor = glGetUniformLocation(gModernGLShader.program, "uLightColor");
    gModernGLShader.uNumLights = glGetUniformLocation(gModernGLShader.program, "uNumLights");
    gModernGLShader.uFogEnabled = glGetUniformLocation(gModernGLShader.program, "uFogEnabled");
    gModernGLShader.uFogStart = glGetUniformLocation(gModernGLShader.program, "uFogStart");
    gModernGLShader.uFogEnd = glGetUniformLocation(gModernGLShader.program, "uFogEnd");
    gModernGLShader.uFogDensity = glGetUniformLocation(gModernGLShader.program, "uFogDensity");
    gModernGLShader.uFogMode = glGetUniformLocation(gModernGLShader.program, "uFogMode");
    gModernGLShader.uFogColor = glGetUniformLocation(gModernGLShader.program, "uFogColor");
    gModernGLShader.uMaterialColor = glGetUniformLocation(gModernGLShader.program, "uMaterialColor");
    gModernGLShader.uUseLighting = glGetUniformLocation(gModernGLShader.program, "uUseLighting");
    gModernGLShader.uUseVertexColor = glGetUniformLocation(gModernGLShader.program, "uUseVertexColor");
    gModernGLShader.uUseTexture0 = glGetUniformLocation(gModernGLShader.program, "uUseTexture0");
    gModernGLShader.uUseTexture1 = glGetUniformLocation(gModernGLShader.program, "uUseTexture1");
    gModernGLShader.uTextureMatrix = glGetUniformLocation(gModernGLShader.program, "uTextureMatrix");
    gModernGLShader.uTexture0 = glGetUniformLocation(gModernGLShader.program, "uTexture0");
    gModernGLShader.uTexture1 = glGetUniformLocation(gModernGLShader.program, "uTexture1");
    gModernGLShader.uMultiTextureMode = glGetUniformLocation(gModernGLShader.program, "uMultiTextureMode");
    gModernGLShader.uMultiTextureCombine = glGetUniformLocation(gModernGLShader.program, "uMultiTextureCombine");
    gModernGLShader.uUseSphereMap = glGetUniformLocation(gModernGLShader.program, "uUseSphereMap");
    gModernGLShader.uAlphaTestEnabled = glGetUniformLocation(gModernGLShader.program, "uAlphaTestEnabled");
    gModernGLShader.uAlphaFunc = glGetUniformLocation(gModernGLShader.program, "uAlphaFunc");
    gModernGLShader.uAlphaRef = glGetUniformLocation(gModernGLShader.program, "uAlphaRef");
    gModernGLShader.uGlobalTransparency = glGetUniformLocation(gModernGLShader.program, "uGlobalTransparency");
    gModernGLShader.uGlobalColorFilter = glGetUniformLocation(gModernGLShader.program, "uGlobalColorFilter");

    printf("[ModernGL] Shaders loaded successfully\n");
    return true;
}

void ModernGL_UseShader(void)
{
    glUseProgram(gModernGLShader.program);
}

void ModernGL_UpdateUniforms(void)
{
    // Upload all state to shader uniforms
    glUniformMatrix4fv(gModernGLShader.uMVPMatrix, 1, GL_FALSE, gModernGLState.mvpMatrix);
    glUniformMatrix4fv(gModernGLShader.uModelViewMatrix, 1, GL_FALSE, gModernGLState.modelViewMatrix);
    glUniformMatrix3fv(gModernGLShader.uNormalMatrix, 1, GL_FALSE, gModernGLState.normalMatrix);
    glUniform3fv(gModernGLShader.uAmbientLight, 1, gModernGLState.ambientLight);
    glUniform3fv(gModernGLShader.uLightDirection, MAX_LIGHTS, (const GLfloat*)gModernGLState.lightDirection);
    glUniform3fv(gModernGLShader.uLightColor, MAX_LIGHTS, (const GLfloat*)gModernGLState.lightColor);
    glUniform1i(gModernGLShader.uNumLights, gModernGLState.numLights);
    glUniform1i(gModernGLShader.uFogEnabled, gModernGLState.fogEnabled);
    glUniform1f(gModernGLShader.uFogStart, gModernGLState.fogStart);
    glUniform1f(gModernGLShader.uFogEnd, gModernGLState.fogEnd);
    glUniform1f(gModernGLShader.uFogDensity, gModernGLState.fogDensity);
    glUniform1i(gModernGLShader.uFogMode, gModernGLState.fogMode);
    glUniform3fv(gModernGLShader.uFogColor, 1, gModernGLState.fogColor);
    glUniform4fv(gModernGLShader.uMaterialColor, 1, gModernGLState.materialColor);
    glUniform1i(gModernGLShader.uUseLighting, gModernGLState.useLighting);
    glUniform1i(gModernGLShader.uUseVertexColor, gModernGLState.useVertexColor);
    glUniform1i(gModernGLShader.uUseTexture0, gModernGLState.useTexture0);
    glUniform1i(gModernGLShader.uUseTexture1, gModernGLState.useTexture1);
    glUniformMatrix4fv(gModernGLShader.uTextureMatrix, 1, GL_FALSE, gModernGLState.textureMatrix);
    glUniform1i(gModernGLShader.uTexture0, 0);
    glUniform1i(gModernGLShader.uTexture1, 1);
    glUniform1i(gModernGLShader.uMultiTextureMode, gModernGLState.multiTextureMode);
    glUniform1i(gModernGLShader.uMultiTextureCombine, gModernGLState.multiTextureCombine);
    glUniform1i(gModernGLShader.uUseSphereMap, gModernGLState.useSphereMap);
    glUniform1i(gModernGLShader.uAlphaTestEnabled, gModernGLState.alphaTestEnabled);
    glUniform1i(gModernGLShader.uAlphaFunc, gModernGLState.alphaFunc);
    glUniform1f(gModernGLShader.uAlphaRef, gModernGLState.alphaRef);
    glUniform1f(gModernGLShader.uGlobalTransparency, gModernGLState.globalTransparency);
    glUniform3fv(gModernGLShader.uGlobalColorFilter, 1, gModernGLState.globalColorFilter);
}

// Geometry management functions
ModernGLGeometry* ModernGL_CreateGeometry(int numVertices, int numIndices, Boolean dynamic)
{
    ModernGLGeometry* geom = (ModernGLGeometry*)malloc(sizeof(ModernGLGeometry));
    memset(geom, 0, sizeof(ModernGLGeometry));

    geom->numVertices = numVertices;
    geom->numIndices = numIndices;
    geom->dynamic = dynamic;
    geom->needsUpload = true;

    // Allocate CPU buffers
    geom->positions = (GLfloat*)malloc(numVertices * 3 * sizeof(GLfloat));
    geom->normals = (GLfloat*)malloc(numVertices * 3 * sizeof(GLfloat));
    geom->colors = (GLfloat*)malloc(numVertices * 4 * sizeof(GLfloat));
    geom->texCoords0 = (GLfloat*)malloc(numVertices * 2 * sizeof(GLfloat));
    geom->texCoords1 = (GLfloat*)malloc(numVertices * 2 * sizeof(GLfloat));
    if (numIndices > 0)
        geom->indices = (GLuint*)malloc(numIndices * sizeof(GLuint));

    // Create GPU buffers
    glGenBuffers(1, &geom->vbo);
    if (numIndices > 0)
        glGenBuffers(1, &geom->ibo);

    return geom;
}

void ModernGL_UploadGeometry(ModernGLGeometry* geom)
{
    // For now, we'll use separate VBOs for each attribute
    // A more optimized approach would interleave the data
    // But this is simpler for the initial implementation

    // We'll pack all vertex data into a single VBO with this layout:
    // positions (3 floats per vertex) + normals (3) + colors (4) + texCoords0 (2) + texCoords1 (2)
    // Total: 14 floats per vertex

    int floatsPerVertex = 14;
    int totalFloats = geom->numVertices * floatsPerVertex;
    GLfloat* interleavedData = (GLfloat*)malloc(totalFloats * sizeof(GLfloat));

    for (int i = 0; i < geom->numVertices; i++)
    {
        int offset = i * floatsPerVertex;
        // Position
        interleavedData[offset + 0] = geom->positions[i * 3 + 0];
        interleavedData[offset + 1] = geom->positions[i * 3 + 1];
        interleavedData[offset + 2] = geom->positions[i * 3 + 2];
        // Normal
        interleavedData[offset + 3] = geom->normals[i * 3 + 0];
        interleavedData[offset + 4] = geom->normals[i * 3 + 1];
        interleavedData[offset + 5] = geom->normals[i * 3 + 2];
        // Color
        interleavedData[offset + 6] = geom->colors[i * 4 + 0];
        interleavedData[offset + 7] = geom->colors[i * 4 + 1];
        interleavedData[offset + 8] = geom->colors[i * 4 + 2];
        interleavedData[offset + 9] = geom->colors[i * 4 + 3];
        // TexCoord0
        interleavedData[offset + 10] = geom->texCoords0[i * 2 + 0];
        interleavedData[offset + 11] = geom->texCoords0[i * 2 + 1];
        // TexCoord1
        interleavedData[offset + 12] = geom->texCoords1[i * 2 + 0];
        interleavedData[offset + 13] = geom->texCoords1[i * 2 + 1];
    }

    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
    GLenum usage = geom->dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    glBufferData(GL_ARRAY_BUFFER, totalFloats * sizeof(GLfloat), interleavedData, usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    free(interleavedData);

    // Upload index buffer if present
    if (geom->numIndices > 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, geom->numIndices * sizeof(GLuint), geom->indices, usage);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    geom->needsUpload = false;
}

void ModernGL_DrawGeometry(ModernGLGeometry* geom, GLenum mode)
{
    if (geom->needsUpload)
        ModernGL_UploadGeometry(geom);

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);

    // Set up vertex attribute pointers (interleaved format: 14 floats per vertex)
    int stride = 14 * sizeof(GLfloat);
    glVertexAttribPointer(ATTRIB_LOCATION_POSITION, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glVertexAttribPointer(ATTRIB_LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(ATTRIB_LOCATION_COLOR, 4, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(GLfloat)));
    glVertexAttribPointer(ATTRIB_LOCATION_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, stride, (void*)(10 * sizeof(GLfloat)));
    glVertexAttribPointer(ATTRIB_LOCATION_TEXCOORD1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(12 * sizeof(GLfloat)));

    // Enable vertex attributes
    glEnableVertexAttribArray(ATTRIB_LOCATION_POSITION);
    glEnableVertexAttribArray(ATTRIB_LOCATION_NORMAL);
    glEnableVertexAttribArray(ATTRIB_LOCATION_COLOR);
    glEnableVertexAttribArray(ATTRIB_LOCATION_TEXCOORD0);
    glEnableVertexAttribArray(ATTRIB_LOCATION_TEXCOORD1);

    // Draw
    if (geom->numIndices > 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->ibo);
        glDrawElements(mode, geom->numIndices, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else
    {
        glDrawArrays(mode, 0, geom->numVertices);
    }

    // Disable vertex attributes
    glDisableVertexAttribArray(ATTRIB_LOCATION_POSITION);
    glDisableVertexAttribArray(ATTRIB_LOCATION_NORMAL);
    glDisableVertexAttribArray(ATTRIB_LOCATION_COLOR);
    glDisableVertexAttribArray(ATTRIB_LOCATION_TEXCOORD0);
    glDisableVertexAttribArray(ATTRIB_LOCATION_TEXCOORD1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ModernGL_FreeGeometry(ModernGLGeometry* geom)
{
    if (geom->vbo)
        glDeleteBuffers(1, &geom->vbo);
    if (geom->ibo)
        glDeleteBuffers(1, &geom->ibo);

    free(geom->positions);
    free(geom->normals);
    free(geom->colors);
    free(geom->texCoords0);
    free(geom->texCoords1);
    if (geom->indices)
        free(geom->indices);

    free(geom);
}

// State management functions
void ModernGL_SetMaterial(float r, float g, float b, float a)
{
    gModernGLState.materialColor[0] = r;
    gModernGLState.materialColor[1] = g;
    gModernGLState.materialColor[2] = b;
    gModernGLState.materialColor[3] = a;
}

void ModernGL_SetLighting(Boolean enabled)
{
    gModernGLState.useLighting = enabled;
}

void ModernGL_SetAmbientLight(float r, float g, float b)
{
    gModernGLState.ambientLight[0] = r;
    gModernGLState.ambientLight[1] = g;
    gModernGLState.ambientLight[2] = b;
}

void ModernGL_SetLight(int lightIndex, float dirX, float dirY, float dirZ, float r, float g, float b)
{
    if (lightIndex >= 0 && lightIndex < MAX_LIGHTS)
    {
        gModernGLState.lightDirection[lightIndex][0] = dirX;
        gModernGLState.lightDirection[lightIndex][1] = dirY;
        gModernGLState.lightDirection[lightIndex][2] = dirZ;
        gModernGLState.lightColor[lightIndex][0] = r;
        gModernGLState.lightColor[lightIndex][1] = g;
        gModernGLState.lightColor[lightIndex][2] = b;
        if (lightIndex >= gModernGLState.numLights)
            gModernGLState.numLights = lightIndex + 1;
    }
}

void ModernGL_SetFog(Boolean enabled, int mode, float start, float end, float density, float r, float g, float b)
{
    gModernGLState.fogEnabled = enabled;
    gModernGLState.fogMode = mode;
    gModernGLState.fogStart = start;
    gModernGLState.fogEnd = end;
    gModernGLState.fogDensity = density;
    gModernGLState.fogColor[0] = r;
    gModernGLState.fogColor[1] = g;
    gModernGLState.fogColor[2] = b;
}

void ModernGL_SetAlphaTest(Boolean enabled, int func, float ref)
{
    gModernGLState.alphaTestEnabled = enabled;
    gModernGLState.alphaFunc = func;
    gModernGLState.alphaRef = ref;
}

void ModernGL_SetMatrices(const float* mvp, const float* modelView, const float* normal)
{
    memcpy(gModernGLState.mvpMatrix, mvp, 16 * sizeof(float));
    memcpy(gModernGLState.modelViewMatrix, modelView, 16 * sizeof(float));
    memcpy(gModernGLState.normalMatrix, normal, 9 * sizeof(float));
}

void ModernGL_SetTextureMatrix(const float* texMatrix)
{
    memcpy(gModernGLState.textureMatrix, texMatrix, 16 * sizeof(float));
}

void ModernGL_SetTextures(Boolean tex0, Boolean tex1, Boolean sphereMap)
{
    gModernGLState.useTexture0 = tex0;
    gModernGLState.useTexture1 = tex1;
    gModernGLState.useSphereMap = sphereMap;
}

void ModernGL_SetMultiTexture(int mode, int combine)
{
    gModernGLState.multiTextureMode = mode;
    gModernGLState.multiTextureCombine = combine;
}

// Immediate mode emulation
void ModernGL_BeginImmediateMode(GLenum mode)
{
    gImmediateModeBuffer.mode = mode;
    gImmediateModeBuffer.vertexCount = 0;
}

void ModernGL_EndImmediateMode(void)
{
    if (gImmediateModeBuffer.vertexCount == 0)
        return;

    // Update shader state before drawing
    extern void CompatGL_UpdateShaderState(void);
    CompatGL_UpdateShaderState();

    GLenum drawMode = gImmediateModeBuffer.mode;
    int numVertices = gImmediateModeBuffer.vertexCount;

    // Convert GL_QUADS to GL_TRIANGLES (WebGL doesn't support quads)
    if (gImmediateModeBuffer.mode == GL_QUADS)
    {
        // Each quad (4 vertices) becomes 2 triangles (6 vertices)
        int numQuads = gImmediateModeBuffer.vertexCount / 4;
        numVertices = numQuads * 6;
        drawMode = GL_TRIANGLES;

        // Create temporary geometry with expanded vertices
        ModernGLGeometry* geom = ModernGL_CreateGeometry(numVertices, 0, true);

        // Convert quads to triangles: 0,1,2,3 -> 0,1,2, 0,2,3
        for (int q = 0; q < numQuads; q++)
        {
            int srcIdx = q * 4;
            int dstIdx = q * 6;

            // First triangle: 0,1,2
            for (int v = 0; v < 3; v++)
            {
                int src = srcIdx + v;
                int dst = dstIdx + v;
                memcpy(&geom->positions[dst * 3], &gImmediateModeBuffer.positions[src * 3], 3 * sizeof(GLfloat));
                memcpy(&geom->normals[dst * 3], &gImmediateModeBuffer.normals[src * 3], 3 * sizeof(GLfloat));
                memcpy(&geom->colors[dst * 4], &gImmediateModeBuffer.colors[src * 4], 4 * sizeof(GLfloat));
                memcpy(&geom->texCoords0[dst * 2], &gImmediateModeBuffer.texCoords[src * 2], 2 * sizeof(GLfloat));
                memcpy(&geom->texCoords1[dst * 2], &gImmediateModeBuffer.texCoords[src * 2], 2 * sizeof(GLfloat));
            }

            // Second triangle: 0,2,3
            int indices[3] = {0, 2, 3};
            for (int v = 0; v < 3; v++)
            {
                int src = srcIdx + indices[v];
                int dst = dstIdx + 3 + v;
                memcpy(&geom->positions[dst * 3], &gImmediateModeBuffer.positions[src * 3], 3 * sizeof(GLfloat));
                memcpy(&geom->normals[dst * 3], &gImmediateModeBuffer.normals[src * 3], 3 * sizeof(GLfloat));
                memcpy(&geom->colors[dst * 4], &gImmediateModeBuffer.colors[src * 4], 4 * sizeof(GLfloat));
                memcpy(&geom->texCoords0[dst * 2], &gImmediateModeBuffer.texCoords[src * 2], 2 * sizeof(GLfloat));
                memcpy(&geom->texCoords1[dst * 2], &gImmediateModeBuffer.texCoords[src * 2], 2 * sizeof(GLfloat));
            }
        }

        ModernGL_DrawGeometry(geom, drawMode);
        ModernGL_FreeGeometry(geom);
    }
    // Convert GL_LINE_LOOP to GL_LINE_STRIP (add first vertex at end)
    else if (gImmediateModeBuffer.mode == GL_LINE_LOOP)
    {
        numVertices = gImmediateModeBuffer.vertexCount + 1;
        drawMode = GL_LINE_STRIP;

        ModernGLGeometry* geom = ModernGL_CreateGeometry(numVertices, 0, true);

        // Copy all vertices
        memcpy(geom->positions, gImmediateModeBuffer.positions, gImmediateModeBuffer.vertexCount * 3 * sizeof(GLfloat));
        memcpy(geom->normals, gImmediateModeBuffer.normals, gImmediateModeBuffer.vertexCount * 3 * sizeof(GLfloat));
        memcpy(geom->colors, gImmediateModeBuffer.colors, gImmediateModeBuffer.vertexCount * 4 * sizeof(GLfloat));
        memcpy(geom->texCoords0, gImmediateModeBuffer.texCoords, gImmediateModeBuffer.vertexCount * 2 * sizeof(GLfloat));
        memcpy(geom->texCoords1, gImmediateModeBuffer.texCoords, gImmediateModeBuffer.vertexCount * 2 * sizeof(GLfloat));

        // Add first vertex at end to close the loop
        int lastIdx = gImmediateModeBuffer.vertexCount;
        memcpy(&geom->positions[lastIdx * 3], &gImmediateModeBuffer.positions[0], 3 * sizeof(GLfloat));
        memcpy(&geom->normals[lastIdx * 3], &gImmediateModeBuffer.normals[0], 3 * sizeof(GLfloat));
        memcpy(&geom->colors[lastIdx * 4], &gImmediateModeBuffer.colors[0], 4 * sizeof(GLfloat));
        memcpy(&geom->texCoords0[lastIdx * 2], &gImmediateModeBuffer.texCoords[0], 2 * sizeof(GLfloat));
        memcpy(&geom->texCoords1[lastIdx * 2], &gImmediateModeBuffer.texCoords[0], 2 * sizeof(GLfloat));

        ModernGL_DrawGeometry(geom, drawMode);
        ModernGL_FreeGeometry(geom);
    }
    // Other modes (GL_TRIANGLES, GL_LINES, GL_LINE_STRIP, etc.) work as-is
    else
    {
        ModernGLGeometry* geom = ModernGL_CreateGeometry(numVertices, 0, true);

        memcpy(geom->positions, gImmediateModeBuffer.positions, numVertices * 3 * sizeof(GLfloat));
        memcpy(geom->normals, gImmediateModeBuffer.normals, numVertices * 3 * sizeof(GLfloat));
        memcpy(geom->colors, gImmediateModeBuffer.colors, numVertices * 4 * sizeof(GLfloat));
        memcpy(geom->texCoords0, gImmediateModeBuffer.texCoords, numVertices * 2 * sizeof(GLfloat));
        memcpy(geom->texCoords1, gImmediateModeBuffer.texCoords, numVertices * 2 * sizeof(GLfloat));

        ModernGL_DrawGeometry(geom, drawMode);
        ModernGL_FreeGeometry(geom);
    }
}

void ModernGL_ImmediateColor(float r, float g, float b, float a)
{
    gImmediateModeBuffer.currentColor[0] = r;
    gImmediateModeBuffer.currentColor[1] = g;
    gImmediateModeBuffer.currentColor[2] = b;
    gImmediateModeBuffer.currentColor[3] = a;
}

void ModernGL_ImmediateNormal(float x, float y, float z)
{
    gImmediateModeBuffer.currentNormal[0] = x;
    gImmediateModeBuffer.currentNormal[1] = y;
    gImmediateModeBuffer.currentNormal[2] = z;
}

void ModernGL_ImmediateTexCoord(float u, float v)
{
    gImmediateModeBuffer.currentTexCoord[0] = u;
    gImmediateModeBuffer.currentTexCoord[1] = v;
}

void ModernGL_ImmediateVertex(float x, float y, float z)
{
    if (gImmediateModeBuffer.vertexCount >= gImmediateModeBuffer.capacity)
    {
        // Expand buffer
        gImmediateModeBuffer.capacity *= 2;
        gImmediateModeBuffer.positions = (GLfloat*)realloc(gImmediateModeBuffer.positions, gImmediateModeBuffer.capacity * 3 * sizeof(GLfloat));
        gImmediateModeBuffer.normals = (GLfloat*)realloc(gImmediateModeBuffer.normals, gImmediateModeBuffer.capacity * 3 * sizeof(GLfloat));
        gImmediateModeBuffer.colors = (GLfloat*)realloc(gImmediateModeBuffer.colors, gImmediateModeBuffer.capacity * 4 * sizeof(GLfloat));
        gImmediateModeBuffer.texCoords = (GLfloat*)realloc(gImmediateModeBuffer.texCoords, gImmediateModeBuffer.capacity * 2 * sizeof(GLfloat));
    }

    int idx = gImmediateModeBuffer.vertexCount;

    // Store position
    gImmediateModeBuffer.positions[idx * 3 + 0] = x;
    gImmediateModeBuffer.positions[idx * 3 + 1] = y;
    gImmediateModeBuffer.positions[idx * 3 + 2] = z;

    // Store normal
    gImmediateModeBuffer.normals[idx * 3 + 0] = gImmediateModeBuffer.currentNormal[0];
    gImmediateModeBuffer.normals[idx * 3 + 1] = gImmediateModeBuffer.currentNormal[1];
    gImmediateModeBuffer.normals[idx * 3 + 2] = gImmediateModeBuffer.currentNormal[2];

    // Store color
    gImmediateModeBuffer.colors[idx * 4 + 0] = gImmediateModeBuffer.currentColor[0];
    gImmediateModeBuffer.colors[idx * 4 + 1] = gImmediateModeBuffer.currentColor[1];
    gImmediateModeBuffer.colors[idx * 4 + 2] = gImmediateModeBuffer.currentColor[2];
    gImmediateModeBuffer.colors[idx * 4 + 3] = gImmediateModeBuffer.currentColor[3];

    // Store texcoord
    gImmediateModeBuffer.texCoords[idx * 2 + 0] = gImmediateModeBuffer.currentTexCoord[0];
    gImmediateModeBuffer.texCoords[idx * 2 + 1] = gImmediateModeBuffer.currentTexCoord[1];

    gImmediateModeBuffer.vertexCount++;
}

// Matrix helper functions
void ModernGL_Matrix4x4ToFloat16(const OGLMatrix4x4* in, float* out)
{
    for (int i = 0; i < 16; i++)
        out[i] = in->value[i];
}

void ModernGL_Matrix3x3FromMatrix4x4(const OGLMatrix4x4* in, float* out)
{
    // Extract 3x3 rotation/scale part from 4x4 matrix
    out[0] = in->value[M00]; out[1] = in->value[M10]; out[2] = in->value[M20];
    out[3] = in->value[M01]; out[4] = in->value[M11]; out[5] = in->value[M21];
    out[6] = in->value[M02]; out[7] = in->value[M12]; out[8] = in->value[M22];
}

#endif // __EMSCRIPTEN__
