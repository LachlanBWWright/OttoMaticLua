//
// nds_performance.h
// NDS Performance Optimization Systems
//
// Provides draw distance culling, terrain LOD, model LOD, and density reduction
// to keep the game within NDS hardware limits:
//   - 4MB RAM
//   - ~6144 polygons per frame
//   - 67MHz ARM9 CPU
//

#pragma once

#include <stdint.h>

// Forward-declare Boolean if not already available (from Pomme/Carbon)
#ifndef __POMME_TYPES__
#ifndef Boolean
typedef unsigned char Boolean;
#endif
#endif

//=====================================================================
// NDS HARDWARE BUDGET CONSTANTS
//=====================================================================

#define NDS_MAX_POLYS_PER_FRAME     6144
#define NDS_MAX_VERTS_PER_FRAME     6144
#define NDS_RAM_BYTES               (4 * 1024 * 1024)

// Performance tier thresholds
#define NDS_POLY_BUDGET_TERRAIN     2048    // polygons reserved for terrain
#define NDS_POLY_BUDGET_OBJECTS     3072    // polygons reserved for objects
#define NDS_POLY_BUDGET_EFFECTS     512     // polygons reserved for particles/effects
#define NDS_POLY_BUDGET_UI          512     // polygons reserved for UI/HUD

//=====================================================================
// DRAW DISTANCE SYSTEM
//=====================================================================

// Reduced draw distances for NDS (in world units)
#ifdef NDS
    #define NDS_DRAW_DIST_NEAR          800.0f      // full detail
    #define NDS_DRAW_DIST_MID           1600.0f     // medium LOD
    #define NDS_DRAW_DIST_FAR           2400.0f     // low LOD
    #define NDS_DRAW_DIST_CULL          3200.0f     // don't draw beyond this

    // Reduced supertile active range for NDS (original is 9)
    #define NDS_SUPERTILE_ACTIVE_RANGE  4
#else
    #define NDS_DRAW_DIST_NEAR          2000.0f
    #define NDS_DRAW_DIST_MID           4000.0f
    #define NDS_DRAW_DIST_FAR           6000.0f
    #define NDS_DRAW_DIST_CULL          8000.0f

    #ifndef SUPERTILE_ACTIVE_RANGE
    #define SUPERTILE_ACTIVE_RANGE      9       // default from terrain.h
    #endif
    #define NDS_SUPERTILE_ACTIVE_RANGE  SUPERTILE_ACTIVE_RANGE
#endif

//=====================================================================
// TERRAIN LOD SYSTEM
//=====================================================================

// LOD levels for terrain supertiles
enum
{
    TERRAIN_LOD_FULL = 0,       // All triangles rendered (128 tris per supertile)
    TERRAIN_LOD_HALF,           // Every other triangle skipped (64 tris)
    TERRAIN_LOD_QUARTER,        // 1/4 triangles rendered (32 tris)
    TERRAIN_LOD_MINIMAL,        // Minimal mesh - just 2 triangles as a flat quad

    NUM_TERRAIN_LOD_LEVELS
};

typedef struct
{
    int     lodLevel;           // current LOD level for this supertile
    int     triCount;           // actual triangle count at current LOD
    float   distanceToCamera;   // cached distance from camera
} TerrainLODInfo;

//=====================================================================
// MODEL LOD SYSTEM
//=====================================================================

// LOD levels for 3D models
enum
{
    MODEL_LOD_HIGH = 0,         // Full detail model
    MODEL_LOD_MEDIUM,           // Reduced detail (skip every other triangle)
    MODEL_LOD_LOW,              // Minimal detail (bounding box or billboard)
    MODEL_LOD_BILLBOARD,        // Replace with sprite billboard

    NUM_MODEL_LOD_LEVELS
};

typedef struct
{
    int     lodLevel;
    int     maxTriangles;       // max tris to render at this LOD
    float   distThreshold;      // distance at which this LOD activates
} ModelLODConfig;

//=====================================================================
// MODEL DENSITY REDUCTION SYSTEM
//=====================================================================

// Categories for density control
enum
{
    DENSITY_CATEGORY_ESSENTIAL = 0,     // Player, key enemies, objectives - always spawn
    DENSITY_CATEGORY_IMPORTANT,         // Main enemies, humans to rescue
    DENSITY_CATEGORY_DECORATIVE,        // Trees, rocks, plants - reduce heavily
    DENSITY_CATEGORY_EFFECT,            // Particles, sparkles - minimal on NDS

    NUM_DENSITY_CATEGORIES
};

// Density reduction factors (0.0 = spawn none, 1.0 = spawn all)
#ifdef NDS
    #define DENSITY_ESSENTIAL       1.0f
    #define DENSITY_IMPORTANT       0.7f
    #define DENSITY_DECORATIVE      0.3f
    #define DENSITY_EFFECT          0.2f
#else
    #define DENSITY_ESSENTIAL       1.0f
    #define DENSITY_IMPORTANT       1.0f
    #define DENSITY_DECORATIVE      1.0f
    #define DENSITY_EFFECT          1.0f
#endif

//=====================================================================
// GLOBAL STATE
//=====================================================================

typedef struct
{
    // Draw distance
    float       drawDistNear;
    float       drawDistMid;
    float       drawDistFar;
    float       drawDistCull;

    // Polygon budget tracking
    int         polysThisFrame;
    int         polyBudgetTerrain;
    int         polyBudgetObjects;
    int         polyBudgetEffects;
    int         polysDrawnTerrain;
    int         polysDrawnObjects;
    int         polysDrawnEffects;
    Boolean     budgetExceeded;

    // Terrain LOD
    int         terrainLODLevel;        // global override (-1 = use distance-based)
    int         supertileActiveRange;   // how far out to load supertiles

    // Model density
    float       densityFactors[NUM_DENSITY_CATEGORIES];

    // Statistics (for debugging/tuning)
    int         objectsCulledByDistance;
    int         objectsCulledByBudget;
    int         objectsReducedLOD;
    int         supertilesCulledByDistance;
    int         itemsSkippedByDensity;

} NDSPerformanceState;

extern NDSPerformanceState gNDSPerf;

//=====================================================================
// FUNCTION PROTOTYPES
//=====================================================================

// Initialization
void    NDS_InitPerformanceSystem(void);
void    NDS_ResetFrameStats(void);

// Draw distance
Boolean NDS_IsObjectInDrawDistance(float objX, float objZ, float cameraX, float cameraZ);
int     NDS_GetDistanceLODLevel(float distance);
float   NDS_GetObjectDistance(float objX, float objZ, float cameraX, float cameraZ);

// Terrain LOD
int     NDS_GetTerrainLODForDistance(float distance);
int     NDS_GetTerrainTriCount(int lodLevel);
Boolean NDS_ShouldDrawTerrainTri(int lodLevel, int triIndex);

// Model LOD
int     NDS_GetModelLODForDistance(float distance);
int     NDS_GetMaxTrisForLOD(int lodLevel, int originalTriCount);
Boolean NDS_ShouldDrawModelTri(int lodLevel, int triIndex);

// Density control
Boolean NDS_ShouldSpawnItem(int densityCategory, int itemIndex);
int     NDS_GetDensityCategory(int itemType);
void    NDS_SetDensityFactor(int category, float factor);

// Budget management
Boolean NDS_CanDrawPolys(int polyCount, int budgetCategory);
void    NDS_AddDrawnPolys(int polyCount, int budgetCategory);
Boolean NDS_IsBudgetExceeded(void);

