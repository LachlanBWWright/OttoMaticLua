//
// vertex_array_compat.c
// Client-side vertex array compatibility implementation for WebGL
//

#ifdef __EMSCRIPTEN__

#include "game.h"
#include <string.h>

VertexArrayState gVertexArrayState;
static int gCurrentClientTexture = 0; // 0 or 1 for GL_TEXTURE0 or GL_TEXTURE1

// Capacity tracker for gVertexArrayState.geometry.
// We only reallocate when the vertex count exceeds the current capacity,
// not on every call, to avoid constant glGenBuffers/glDeleteBuffers churn.
static int gVertexArrayGeomCapacity = 0;

// Persistent scratch geometry used as the destination for expanded index draws.
// Reused across frames; only grows, never shrinks, to avoid per-draw-call
// glGenBuffers / glDeleteBuffers which stall the GPU pipeline.
static ModernGLGeometry* gExpandedScratch = NULL;
static int gExpandedScratchCapacity = 0;

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
    // Only reallocate when the required count exceeds the current capacity.
    // Avoiding glGenBuffers/glDeleteBuffers on every draw call is the single
    // biggest GPU-pipeline performance win: constant reallocation causes the
    // driver to stall while it waits for in-flight GPU work to complete.
    if (!gVertexArrayState.geometry || vertexCount > gVertexArrayGeomCapacity)
    {
        if (gVertexArrayState.geometry)
            ModernGL_FreeGeometry(gVertexArrayState.geometry);

        gVertexArrayState.geometry = ModernGL_CreateGeometry(vertexCount, 0, true);
        gVertexArrayGeomCapacity = vertexCount;
    }
    gVertexArrayState.geometry->numVertices = vertexCount;

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
    // Sync vertex color state to shader
    extern ModernGLState gModernGLState;
    gModernGLState.useVertexColor = gVertexArrayState.colorArrayEnabled;

    // Update shader state before drawing
    extern void CompatGL_UpdateShaderState(void);
    CompatGL_UpdateShaderState();

    if (type != GL_UNSIGNED_INT && type != GL_UNSIGNED_SHORT)
        return; // Unsupported index type

    // Find the maximum index to know how many source vertices to convert
    GLuint maxIdx = 0;
    if (type == GL_UNSIGNED_INT)
    {
        const GLuint* idx = (const GLuint*)indices;
        for (int i = 0; i < count; i++)
            if (idx[i] > maxIdx) maxIdx = idx[i];
    }
    else
    {
        const GLushort* idx = (const GLushort*)indices;
        for (int i = 0; i < count; i++)
            if ((GLuint)idx[i] > maxIdx) maxIdx = idx[i];
    }

    // Convert source vertex arrays (maxIdx+1 vertices needed)
    ConvertVertexArraysToVBO(maxIdx + 1);

    // Expand vertices based on indices into a persistent scratch geometry.
    // Using a reusable geometry avoids per-draw-call glGenBuffers/glDeleteBuffers,
    // which would otherwise stall the GPU pipeline on every draw call.
    if (!gExpandedScratch || count > gExpandedScratchCapacity)
    {
        if (gExpandedScratch) ModernGL_FreeGeometry(gExpandedScratch);
        gExpandedScratch = ModernGL_CreateGeometry(count, 0, true);
        gExpandedScratchCapacity = count;
    }
    gExpandedScratch->numVertices = count;
    gExpandedScratch->needsUpload = true;

    ModernGLGeometry* src = gVertexArrayState.geometry;
    ModernGLGeometry* expanded = gExpandedScratch;

    if (type == GL_UNSIGNED_INT)
    {
        const GLuint* idx = (const GLuint*)indices;
        for (int i = 0; i < count; i++)
        {
            int srcIdx = idx[i];
            memcpy(&expanded->positions[i * 3], &src->positions[srcIdx * 3], 3 * sizeof(GLfloat));
            memcpy(&expanded->normals[i * 3], &src->normals[srcIdx * 3], 3 * sizeof(GLfloat));
            memcpy(&expanded->colors[i * 4], &src->colors[srcIdx * 4], 4 * sizeof(GLfloat));
            memcpy(&expanded->texCoords0[i * 2], &src->texCoords0[srcIdx * 2], 2 * sizeof(GLfloat));
            memcpy(&expanded->texCoords1[i * 2], &src->texCoords1[srcIdx * 2], 2 * sizeof(GLfloat));
        }
    }
    else
    {
        const GLushort* idx = (const GLushort*)indices;
        for (int i = 0; i < count; i++)
        {
            int srcIdx = idx[i];
            memcpy(&expanded->positions[i * 3], &src->positions[srcIdx * 3], 3 * sizeof(GLfloat));
            memcpy(&expanded->normals[i * 3], &src->normals[srcIdx * 3], 3 * sizeof(GLfloat));
            memcpy(&expanded->colors[i * 4], &src->colors[srcIdx * 4], 4 * sizeof(GLfloat));
            memcpy(&expanded->texCoords0[i * 2], &src->texCoords0[srcIdx * 2], 2 * sizeof(GLfloat));
            memcpy(&expanded->texCoords1[i * 2], &src->texCoords1[srcIdx * 2], 2 * sizeof(GLfloat));
        }
    }

    ModernGL_DrawGeometry(expanded, mode);
}

void CompatGL_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    // Sync vertex color state to shader
    extern ModernGLState gModernGLState;
    gModernGLState.useVertexColor = gVertexArrayState.colorArrayEnabled;

    // Update shader state before drawing
    extern void CompatGL_UpdateShaderState(void);
    CompatGL_UpdateShaderState();

    // Convert only the vertices in [first, first+count) directly into
    // gVertexArrayState.geometry with 'count' entries, then draw from it.
    // This avoids the old approach of converting first+count vertices and
    // then making a second, per-draw-call subset copy (which created and
    // destroyed a VBO on every single draw call).

    // Only reallocate when we need more capacity (don't recreate on every frame)
    if (!gVertexArrayState.geometry || count > gVertexArrayGeomCapacity)
    {
        if (gVertexArrayState.geometry)
            ModernGL_FreeGeometry(gVertexArrayState.geometry);

        gVertexArrayState.geometry = ModernGL_CreateGeometry(count, 0, true);
        gVertexArrayGeomCapacity = count;
    }
    gVertexArrayState.geometry->numVertices = count;

    ModernGLGeometry* geom = gVertexArrayState.geometry;

    // Convert vertex positions from the [first, first+count) range
    if (gVertexArrayState.vertexArrayEnabled && gVertexArrayState.vertexPointer)
    {
        const GLfloat* src = (const GLfloat*)gVertexArrayState.vertexPointer + first * 3;
        memcpy(geom->positions, src, count * 3 * sizeof(GLfloat));
    }

    // Convert normals from the [first, first+count) range
    if (gVertexArrayState.normalArrayEnabled && gVertexArrayState.normalPointer)
    {
        const GLfloat* src = (const GLfloat*)gVertexArrayState.normalPointer + first * 3;
        memcpy(geom->normals, src, count * 3 * sizeof(GLfloat));
    }
    else
    {
        // Default normals (pointing up)
        for (int i = 0; i < count; i++)
        {
            geom->normals[i * 3 + 0] = 0.0f;
            geom->normals[i * 3 + 1] = 1.0f;
            geom->normals[i * 3 + 2] = 0.0f;
        }
    }

    // Convert colors from the [first, first+count) range
    if (gVertexArrayState.colorArrayEnabled && gVertexArrayState.colorPointer)
    {
        if (gVertexArrayState.colorType == GL_FLOAT)
        {
            const GLfloat* src = (const GLfloat*)gVertexArrayState.colorPointer + first * 4;
            memcpy(geom->colors, src, count * 4 * sizeof(GLfloat));
        }
        else if (gVertexArrayState.colorType == GL_UNSIGNED_BYTE)
        {
            const GLubyte* src = (const GLubyte*)gVertexArrayState.colorPointer + first * 4;
            for (int i = 0; i < count; i++)
            {
                geom->colors[i * 4 + 0] = src[i * 4 + 0] / 255.0f;
                geom->colors[i * 4 + 1] = src[i * 4 + 1] / 255.0f;
                geom->colors[i * 4 + 2] = src[i * 4 + 2] / 255.0f;
                geom->colors[i * 4 + 3] = src[i * 4 + 3] / 255.0f;
            }
        }
    }
    else
    {
        // Default color (white)
        for (int i = 0; i < count; i++)
        {
            geom->colors[i * 4 + 0] = 1.0f;
            geom->colors[i * 4 + 1] = 1.0f;
            geom->colors[i * 4 + 2] = 1.0f;
            geom->colors[i * 4 + 3] = 1.0f;
        }
    }

    // Convert texture coordinates from the [first, first+count) range
    for (int texUnit = 0; texUnit < 2; texUnit++)
    {
        GLfloat* dst = (texUnit == 0) ? geom->texCoords0 : geom->texCoords1;

        if (gVertexArrayState.texCoordArrayEnabled[texUnit] && gVertexArrayState.texCoordPointers[texUnit])
        {
            const GLfloat* src = (const GLfloat*)gVertexArrayState.texCoordPointers[texUnit] + first * 2;
            memcpy(dst, src, count * 2 * sizeof(GLfloat));
        }
        else
        {
            memset(dst, 0, count * 2 * sizeof(GLfloat));
        }
    }

    geom->needsUpload = true;
    ModernGL_DrawGeometry(geom, mode);
}

#endif // __EMSCRIPTEN__
