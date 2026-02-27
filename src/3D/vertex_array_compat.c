//
// vertex_array_compat.c
// Client-side vertex array compatibility implementation for WebGL
//

#ifdef __EMSCRIPTEN__

#include "game.h"
#include <string.h>
#include <stdio.h>

VertexArrayState gVertexArrayState;
static int gCurrentClientTexture = 0; // 0 or 1 for GL_TEXTURE0 or GL_TEXTURE1

void CompatGL_EnableClientState(GLenum array)
{
    switch (array)
    {
        case GL_VERTEX_ARRAY:
            gVertexArrayState.vertexArrayEnabled = true;
            break;
        case GL_NORMAL_ARRAY:
            gVertexArrayState.normalArrayEnabled = true;
            break;
        case GL_COLOR_ARRAY:
            gVertexArrayState.colorArrayEnabled = true;
            break;
        case GL_TEXTURE_COORD_ARRAY:
            if (gCurrentClientTexture >= 0 && gCurrentClientTexture < 2)
                gVertexArrayState.texCoordArrayEnabled[gCurrentClientTexture] = true;
            break;
    }
    gVertexArrayState.isDirty = true;
}

void CompatGL_DisableClientState(GLenum array)
{
    switch (array)
    {
        case GL_VERTEX_ARRAY:
            gVertexArrayState.vertexArrayEnabled = false;
            break;
        case GL_NORMAL_ARRAY:
            gVertexArrayState.normalArrayEnabled = false;
            break;
        case GL_COLOR_ARRAY:
            gVertexArrayState.colorArrayEnabled = false;
            break;
        case GL_TEXTURE_COORD_ARRAY:
            if (gCurrentClientTexture >= 0 && gCurrentClientTexture < 2)
                gVertexArrayState.texCoordArrayEnabled[gCurrentClientTexture] = false;
            break;
    }
    gVertexArrayState.isDirty = true;
}

void CompatGL_VertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    gVertexArrayState.vertexPointer = pointer;
    gVertexArrayState.vertexSize = size;
    gVertexArrayState.vertexType = type;
    gVertexArrayState.vertexStride = stride;
    gVertexArrayState.isDirty = true;
}

void CompatGL_NormalPointer(GLenum type, GLsizei stride, const void* pointer)
{
    gVertexArrayState.normalPointer = pointer;
    gVertexArrayState.isDirty = true;
}

void CompatGL_ColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    gVertexArrayState.colorPointer = pointer;
    gVertexArrayState.colorSize = size;
    gVertexArrayState.colorType = type;
    gVertexArrayState.colorStride = stride;
    gVertexArrayState.isDirty = true;
}

void CompatGL_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    if (gCurrentClientTexture >= 0 && gCurrentClientTexture < 2)
    {
        gVertexArrayState.texCoordPointers[gCurrentClientTexture] = pointer;
        gVertexArrayState.texCoordSize[gCurrentClientTexture] = size;
        gVertexArrayState.texCoordStride[gCurrentClientTexture] = stride;
        gVertexArrayState.isDirty = true;
    }
}

void CompatGL_ClientActiveTexture(GLenum texture)
{
    // Convert GL_TEXTURE0_ARB/GL_TEXTURE1_ARB to index
    if (texture == GL_TEXTURE0_ARB || texture == GL_TEXTURE0)
        gCurrentClientTexture = 0;
    else if (texture == GL_TEXTURE1_ARB || texture == GL_TEXTURE1)
        gCurrentClientTexture = 1;
}

static void ConvertVertexArraysToVBO(int vertexCount)
{
    // Create or reuse geometry
    if (!gVertexArrayState.geometry || gVertexArrayState.geometry->numVertices != vertexCount)
    {
        if (gVertexArrayState.geometry)
            ModernGL_FreeGeometry(gVertexArrayState.geometry);

        gVertexArrayState.geometry = ModernGL_CreateGeometry(vertexCount, 0, true);
    }

    ModernGLGeometry* geom = gVertexArrayState.geometry;

    // Convert vertex positions
    if (gVertexArrayState.vertexArrayEnabled && gVertexArrayState.vertexPointer)
    {
        const GLfloat* src = (const GLfloat*)gVertexArrayState.vertexPointer;
        for (int i = 0; i < vertexCount; i++)
        {
            const GLfloat* vertex = &src[i * 3]; // Assuming 3 components
            geom->positions[i * 3 + 0] = vertex[0];
            geom->positions[i * 3 + 1] = vertex[1];
            geom->positions[i * 3 + 2] = vertex[2];
        }
    }

    // Convert normals
    if (gVertexArrayState.normalArrayEnabled && gVertexArrayState.normalPointer)
    {
        const GLfloat* src = (const GLfloat*)gVertexArrayState.normalPointer;
        for (int i = 0; i < vertexCount; i++)
        {
            const GLfloat* normal = &src[i * 3];
            geom->normals[i * 3 + 0] = normal[0];
            geom->normals[i * 3 + 1] = normal[1];
            geom->normals[i * 3 + 2] = normal[2];
        }
    }
    else
    {
        // Default normals (pointing up)
        for (int i = 0; i < vertexCount; i++)
        {
            geom->normals[i * 3 + 0] = 0.0f;
            geom->normals[i * 3 + 1] = 1.0f;
            geom->normals[i * 3 + 2] = 0.0f;
        }
    }

    // Convert colors
    if (gVertexArrayState.colorArrayEnabled && gVertexArrayState.colorPointer)
    {
        if (gVertexArrayState.colorType == GL_FLOAT)
        {
            const GLfloat* src = (const GLfloat*)gVertexArrayState.colorPointer;
            for (int i = 0; i < vertexCount; i++)
            {
                const GLfloat* color = &src[i * 4];
                geom->colors[i * 4 + 0] = color[0];
                geom->colors[i * 4 + 1] = color[1];
                geom->colors[i * 4 + 2] = color[2];
                geom->colors[i * 4 + 3] = color[3];
            }
        }
        else if (gVertexArrayState.colorType == GL_UNSIGNED_BYTE)
        {
            const GLubyte* src = (const GLubyte*)gVertexArrayState.colorPointer;
            for (int i = 0; i < vertexCount; i++)
            {
                const GLubyte* color = &src[i * 4];
                geom->colors[i * 4 + 0] = color[0] / 255.0f;
                geom->colors[i * 4 + 1] = color[1] / 255.0f;
                geom->colors[i * 4 + 2] = color[2] / 255.0f;
                geom->colors[i * 4 + 3] = color[3] / 255.0f;
            }
        }
    }
    else
    {
        // Default color (white)
        for (int i = 0; i < vertexCount; i++)
        {
            geom->colors[i * 4 + 0] = 1.0f;
            geom->colors[i * 4 + 1] = 1.0f;
            geom->colors[i * 4 + 2] = 1.0f;
            geom->colors[i * 4 + 3] = 1.0f;
        }
    }

    // Convert texture coordinates
    for (int texUnit = 0; texUnit < 2; texUnit++)
    {
        GLfloat* dst = (texUnit == 0) ? geom->texCoords0 : geom->texCoords1;

        if (gVertexArrayState.texCoordArrayEnabled[texUnit] && gVertexArrayState.texCoordPointers[texUnit])
        {
            const GLfloat* src = (const GLfloat*)gVertexArrayState.texCoordPointers[texUnit];
            for (int i = 0; i < vertexCount; i++)
            {
                const GLfloat* texCoord = &src[i * 2];
                dst[i * 2 + 0] = texCoord[0];
                dst[i * 2 + 1] = texCoord[1];
            }
        }
        else
        {
            // Default tex coords (0,0)
            for (int i = 0; i < vertexCount; i++)
            {
                dst[i * 2 + 0] = 0.0f;
                dst[i * 2 + 1] = 0.0f;
            }
        }
    }

    geom->needsUpload = true;
}

void CompatGL_DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
    static int sDrawCallCount = 0;
    sDrawCallCount++;

    // Log first few draw calls for diagnostic purposes
    if (sDrawCallCount <= 3)
    {
        printf("[CompatGL] DrawElements #%d: mode=0x%x count=%d type=0x%x\n",
               sDrawCallCount, mode, count, type);
        printf("[CompatGL]   vertexArray=%d normalArray=%d colorArray=%d texCoord0=%d texCoord1=%d\n",
               gVertexArrayState.vertexArrayEnabled,
               gVertexArrayState.normalArrayEnabled,
               gVertexArrayState.colorArrayEnabled,
               gVertexArrayState.texCoordArrayEnabled[0],
               gVertexArrayState.texCoordArrayEnabled[1]);
    }

    // Sync vertex color state to shader
    extern ModernGLState gModernGLState;
    gModernGLState.useVertexColor = gVertexArrayState.colorArrayEnabled;

    // Update shader state before drawing
    extern void CompatGL_UpdateShaderState(void);
    CompatGL_UpdateShaderState();

    // For now, we'll convert indexed draw to non-indexed by expanding vertices
    // A more optimized approach would use index buffers

    if (type == GL_UNSIGNED_INT)
    {
        const GLuint* idx = (const GLuint*)indices;

        // Find the maximum index to know how many source vertices to convert
        GLuint maxIdx = 0;
        for (int i = 0; i < count; i++)
        {
            if (idx[i] > maxIdx) maxIdx = idx[i];
        }

        // Convert source vertex arrays (maxIdx+1 vertices needed)
        ConvertVertexArraysToVBO(maxIdx + 1);

        // Expand vertices based on indices
        ModernGLGeometry* src = gVertexArrayState.geometry;
        ModernGLGeometry* expanded = ModernGL_CreateGeometry(count, 0, true);

        for (int i = 0; i < count; i++)
        {
            int srcIdx = idx[i];
            memcpy(&expanded->positions[i * 3], &src->positions[srcIdx * 3], 3 * sizeof(GLfloat));
            memcpy(&expanded->normals[i * 3], &src->normals[srcIdx * 3], 3 * sizeof(GLfloat));
            memcpy(&expanded->colors[i * 4], &src->colors[srcIdx * 4], 4 * sizeof(GLfloat));
            memcpy(&expanded->texCoords0[i * 2], &src->texCoords0[srcIdx * 2], 2 * sizeof(GLfloat));
            memcpy(&expanded->texCoords1[i * 2], &src->texCoords1[srcIdx * 2], 2 * sizeof(GLfloat));
        }

        ModernGL_DrawGeometry(expanded, mode);
        ModernGL_FreeGeometry(expanded);
    }
}

void CompatGL_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    // Sync vertex color state to shader
    extern ModernGLState gModernGLState;
    gModernGLState.useVertexColor = gVertexArrayState.colorArrayEnabled;

    // Update shader state before drawing
    extern void CompatGL_UpdateShaderState(void);
    CompatGL_UpdateShaderState();

    // Convert vertex arrays starting from 'first' for 'count' vertices
    // For simplicity, we'll convert the entire array and offset
    ConvertVertexArraysToVBO(first + count);

    // Create a geometry with just the subset we need
    ModernGLGeometry* subset = ModernGL_CreateGeometry(count, 0, true);
    ModernGLGeometry* full = gVertexArrayState.geometry;

    for (int i = 0; i < count; i++)
    {
        int srcIdx = first + i;
        memcpy(&subset->positions[i * 3], &full->positions[srcIdx * 3], 3 * sizeof(GLfloat));
        memcpy(&subset->normals[i * 3], &full->normals[srcIdx * 3], 3 * sizeof(GLfloat));
        memcpy(&subset->colors[i * 4], &full->colors[srcIdx * 4], 4 * sizeof(GLfloat));
        memcpy(&subset->texCoords0[i * 2], &full->texCoords0[srcIdx * 2], 2 * sizeof(GLfloat));
        memcpy(&subset->texCoords1[i * 2], &full->texCoords1[srcIdx * 2], 2 * sizeof(GLfloat));
    }

    ModernGL_DrawGeometry(subset, mode);
    ModernGL_FreeGeometry(subset);
}

#endif // __EMSCRIPTEN__
