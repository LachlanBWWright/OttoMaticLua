/****************************/
/*   NDS_PERFORMANCE.C      */
/*                          */
/* NDS Performance          */
/* Optimization Systems     */
/****************************/

/***************/
/* EXTERNALS   */
/***************/

#include "game.h"
#include "nds_performance.h"

/****************************/
/*    CONSTANTS             */
/****************************/

// Pseudo-random seed for deterministic density reduction
// (so items don't pop in/out between frames)
#define DENSITY_HASH_SEED   0xDEADBEEF

/****************************/
/*    VARIABLES             */
/****************************/

NDSPerformanceState gNDSPerf;


/****************************/
/*    PROTOTYPES            */
/****************************/

static uint32_t DensityHash(int itemIndex);


/*=====================================================================*/
/*                        INITIALIZATION                                */
/*=====================================================================*/


/****************** NDS INIT PERFORMANCE SYSTEM ************************/

void NDS_InitPerformanceSystem(void)
{
    // Draw distance settings
#ifdef NDS
    gNDSPerf.drawDistNear       = NDS_DRAW_DIST_NEAR;
    gNDSPerf.drawDistMid        = NDS_DRAW_DIST_MID;
    gNDSPerf.drawDistFar        = NDS_DRAW_DIST_FAR;
    gNDSPerf.drawDistCull       = NDS_DRAW_DIST_CULL;
    gNDSPerf.supertileActiveRange = NDS_SUPERTILE_ACTIVE_RANGE;
#else
    gNDSPerf.drawDistNear       = NDS_DRAW_DIST_NEAR;
    gNDSPerf.drawDistMid        = NDS_DRAW_DIST_MID;
    gNDSPerf.drawDistFar        = NDS_DRAW_DIST_FAR;
    gNDSPerf.drawDistCull       = NDS_DRAW_DIST_CULL;
    gNDSPerf.supertileActiveRange = SUPERTILE_ACTIVE_RANGE;
#endif

    // Polygon budgets
    gNDSPerf.polyBudgetTerrain  = NDS_POLY_BUDGET_TERRAIN;
    gNDSPerf.polyBudgetObjects  = NDS_POLY_BUDGET_OBJECTS;
    gNDSPerf.polyBudgetEffects  = NDS_POLY_BUDGET_EFFECTS;

    // Terrain LOD
    gNDSPerf.terrainLODLevel    = -1;   // use distance-based by default

    // Density factors
    gNDSPerf.densityFactors[DENSITY_CATEGORY_ESSENTIAL]   = DENSITY_ESSENTIAL;
    gNDSPerf.densityFactors[DENSITY_CATEGORY_IMPORTANT]   = DENSITY_IMPORTANT;
    gNDSPerf.densityFactors[DENSITY_CATEGORY_DECORATIVE]  = DENSITY_DECORATIVE;
    gNDSPerf.densityFactors[DENSITY_CATEGORY_EFFECT]      = DENSITY_EFFECT;

    NDS_ResetFrameStats();
}


/****************** NDS RESET FRAME STATS ************************/

void NDS_ResetFrameStats(void)
{
    gNDSPerf.polysThisFrame         = 0;
    gNDSPerf.polysDrawnTerrain      = 0;
    gNDSPerf.polysDrawnObjects      = 0;
    gNDSPerf.polysDrawnEffects      = 0;
    gNDSPerf.budgetExceeded         = false;
    gNDSPerf.objectsCulledByDistance = 0;
    gNDSPerf.objectsCulledByBudget  = 0;
    gNDSPerf.objectsReducedLOD      = 0;
    gNDSPerf.supertilesCulledByDistance = 0;
    gNDSPerf.itemsSkippedByDensity  = 0;
}


/*=====================================================================*/
/*                      DRAW DISTANCE SYSTEM                            */
/*=====================================================================*/


/****************** NDS IS OBJECT IN DRAW DISTANCE ************************/
//
// Returns true if object is within the max draw distance
//

Boolean NDS_IsObjectInDrawDistance(float objX, float objZ, float cameraX, float cameraZ)
{
    float dx = objX - cameraX;
    float dz = objZ - cameraZ;
    float distSq = dx * dx + dz * dz;
    float cullDistSq = gNDSPerf.drawDistCull * gNDSPerf.drawDistCull;

    if (distSq > cullDistSq)
    {
        gNDSPerf.objectsCulledByDistance++;
        return false;
    }
    return true;
}


/****************** NDS GET DISTANCE LOD LEVEL ************************/
//
// Returns LOD level (0-3) based on distance from camera
//

int NDS_GetDistanceLODLevel(float distance)
{
    if (distance < gNDSPerf.drawDistNear)
        return 0;   // highest detail
    else if (distance < gNDSPerf.drawDistMid)
        return 1;   // medium detail
    else if (distance < gNDSPerf.drawDistFar)
        return 2;   // low detail
    else
        return 3;   // minimal/billboard
}


/****************** NDS GET OBJECT DISTANCE ************************/
//
// Fast approximate distance (no sqrt - uses max of dx,dz + half of min)
//

float NDS_GetObjectDistance(float objX, float objZ, float cameraX, float cameraZ)
{
    float dx = objX - cameraX;
    float dz = objZ - cameraZ;

    if (dx < 0) dx = -dx;
    if (dz < 0) dz = -dz;

    // Approximate distance: max + 0.5*min (within ~12% of true distance)
    if (dx > dz)
        return dx + dz * 0.5f;
    else
        return dz + dx * 0.5f;
}


/*=====================================================================*/
/*                       TERRAIN LOD SYSTEM                             */
/*=====================================================================*/


/****************** NDS GET TERRAIN LOD FOR DISTANCE ************************/
//
// Returns the LOD level for a terrain supertile based on distance
//

int NDS_GetTerrainLODForDistance(float distance)
{
    // If global override is set, use it
    if (gNDSPerf.terrainLODLevel >= 0)
        return gNDSPerf.terrainLODLevel;

    if (distance < gNDSPerf.drawDistNear)
        return TERRAIN_LOD_FULL;
    else if (distance < gNDSPerf.drawDistMid)
        return TERRAIN_LOD_HALF;
    else if (distance < gNDSPerf.drawDistFar)
        return TERRAIN_LOD_QUARTER;
    else
        return TERRAIN_LOD_MINIMAL;
}


/****************** NDS GET TERRAIN TRI COUNT ************************/
//
// Returns the number of triangles to draw for a supertile at a given LOD
//

int NDS_GetTerrainTriCount(int lodLevel)
{
    switch (lodLevel)
    {
        case TERRAIN_LOD_FULL:      return NUM_TRIS_IN_SUPERTILE;       // 128
        case TERRAIN_LOD_HALF:      return NUM_TRIS_IN_SUPERTILE / 2;   // 64
        case TERRAIN_LOD_QUARTER:   return NUM_TRIS_IN_SUPERTILE / 4;   // 32
        case TERRAIN_LOD_MINIMAL:   return 2;                            // just a quad
        default:                    return NUM_TRIS_IN_SUPERTILE;
    }
}


/****************** NDS SHOULD DRAW TERRAIN TRI ************************/
//
// For LOD levels that skip triangles, determines if a specific triangle
// should be drawn. Uses a regular skip pattern to maintain mesh coherence.
//

Boolean NDS_ShouldDrawTerrainTri(int lodLevel, int triIndex)
{
    switch (lodLevel)
    {
        case TERRAIN_LOD_FULL:
            return true;

        case TERRAIN_LOD_HALF:
            // Draw every other triangle (even indices only)
            return (triIndex & 1) == 0;

        case TERRAIN_LOD_QUARTER:
            // Draw every 4th triangle
            return (triIndex & 3) == 0;

        case TERRAIN_LOD_MINIMAL:
            // Only draw first 2 triangles (forms a quad covering the supertile)
            return triIndex < 2;

        default:
            return true;
    }
}


/*=====================================================================*/
/*                        MODEL LOD SYSTEM                              */
/*=====================================================================*/


/****************** NDS GET MODEL LOD FOR DISTANCE ************************/

int NDS_GetModelLODForDistance(float distance)
{
    if (distance < gNDSPerf.drawDistNear)
        return MODEL_LOD_HIGH;
    else if (distance < gNDSPerf.drawDistMid)
        return MODEL_LOD_MEDIUM;
    else if (distance < gNDSPerf.drawDistFar)
        return MODEL_LOD_LOW;
    else
        return MODEL_LOD_BILLBOARD;
}


/****************** NDS GET MAX TRIS FOR LOD ************************/
//
// Returns the maximum number of triangles to render for an object
// at the given LOD level
//

int NDS_GetMaxTrisForLOD(int lodLevel, int originalTriCount)
{
    switch (lodLevel)
    {
        case MODEL_LOD_HIGH:
            return originalTriCount;

        case MODEL_LOD_MEDIUM:
            return originalTriCount / 2;

        case MODEL_LOD_LOW:
            return originalTriCount / 4;

        case MODEL_LOD_BILLBOARD:
            return 2;   // billboard quad

        default:
            return originalTriCount;
    }
}


/****************** NDS SHOULD DRAW MODEL TRI ************************/
//
// Determines whether a specific triangle in a model mesh should be drawn
// at the current LOD level. Uses stride-based skipping.
//

Boolean NDS_ShouldDrawModelTri(int lodLevel, int triIndex)
{
    switch (lodLevel)
    {
        case MODEL_LOD_HIGH:
            return true;

        case MODEL_LOD_MEDIUM:
            return (triIndex & 1) == 0;

        case MODEL_LOD_LOW:
            return (triIndex & 3) == 0;

        case MODEL_LOD_BILLBOARD:
            return triIndex < 2;

        default:
            return true;
    }
}


/*=====================================================================*/
/*                    DENSITY REDUCTION SYSTEM                          */
/*=====================================================================*/


/****************** DENSITY HASH ************************/
//
// Simple hash function for deterministic pseudo-random density reduction.
// Given an item index, returns a pseudo-random value used to decide
// if the item should spawn.
//

static uint32_t DensityHash(int itemIndex)
{
    uint32_t h = (uint32_t)itemIndex ^ DENSITY_HASH_SEED;
    h = (h ^ (h >> 16)) * 0x45d9f3b;
    h = (h ^ (h >> 16)) * 0x45d9f3b;
    h = h ^ (h >> 16);
    return h;
}


/****************** NDS SHOULD SPAWN ITEM ************************/
//
// Determines whether a terrain item should be spawned based on its
// density category. Essential items always spawn. Decorative items
// are heavily reduced on NDS.
//
// The decision is deterministic based on itemIndex so items don't
// randomly appear/disappear between level loads.
//

Boolean NDS_ShouldSpawnItem(int densityCategory, int itemIndex)
{
    float factor;

    // Essential items always spawn
    if (densityCategory == DENSITY_CATEGORY_ESSENTIAL)
        return true;

    // Get density factor for this category
    if (densityCategory < 0 || densityCategory >= NUM_DENSITY_CATEGORIES)
        factor = 1.0f;
    else
        factor = gNDSPerf.densityFactors[densityCategory];

    // If factor is 1.0, always spawn
    if (factor >= 1.0f)
        return true;

    // If factor is 0.0, never spawn
    if (factor <= 0.0f)
    {
        gNDSPerf.itemsSkippedByDensity++;
        return false;
    }

    // Use deterministic hash to decide
    uint32_t hash = DensityHash(itemIndex);
    float normalizedHash = (float)(hash & 0xFFFF) / 65535.0f;

    if (normalizedHash > factor)
    {
        gNDSPerf.itemsSkippedByDensity++;
        return false;
    }

    return true;
}


/****************** NDS GET DENSITY CATEGORY ************************/
//
// Maps a terrain item type to a density category.
// Essential: player start, exit rockets, checkpoints, teleporters
// Important: enemies, humans, powerups
// Decorative: plants, rocks, fences, ambient objects
// Effect: particles, sparkles
//

int NDS_GetDensityCategory(int itemType)
{
    switch (itemType)
    {
        // Essential - never cull these
        case MAP_ITEM_MYSTARTCOORD:
        case MAP_ITEM_EXITROCKET:
        case MAP_ITEM_CHECKPOINT:
        case MAP_ITEM_TELEPORTER:
        case MAP_ITEM_BRAINPORTAL:
        case MAP_ITEM_BUMPERCARPOWER:
        case MAP_ITEM_BUMPERCARGATE:
        case MAP_ITEM_ROCKETSLED:
        case MAP_ITEM_ZIPLINE:
            return DENSITY_CATEGORY_ESSENTIAL;

        // Important - reduce moderately
        case MAP_ITEM_HUMAN:
        case MAP_ITEM_HUMAN_SCIENTIST:
        case MAP_ITEM_ATOM:
        case MAP_ITEM_POWERUPPOD:
        case MAP_ITEM_BUBBLEPUMP:
        case MAP_ITEM_BLOBBOSS:
        case MAP_ITEM_PEOPLE_HUT:
            return DENSITY_CATEGORY_IMPORTANT;

        // Everything else is decorative
        default:
            return DENSITY_CATEGORY_DECORATIVE;
    }
}


/****************** NDS SET DENSITY FACTOR ************************/

void NDS_SetDensityFactor(int category, float factor)
{
    if (category >= 0 && category < NUM_DENSITY_CATEGORIES)
    {
        if (factor < 0.0f) factor = 0.0f;
        if (factor > 1.0f) factor = 1.0f;
        gNDSPerf.densityFactors[category] = factor;
    }
}


/*=====================================================================*/
/*                    POLYGON BUDGET MANAGEMENT                         */
/*=====================================================================*/


/****************** NDS CAN DRAW POLYS ************************/
//
// Checks if drawing the specified number of polygons would exceed
// the budget for the given category. Returns true if OK to draw.
//

Boolean NDS_CanDrawPolys(int polyCount, int budgetCategory)
{
#ifdef NDS
    int budget;
    int drawn;

    switch (budgetCategory)
    {
        case 0: // terrain
            budget = gNDSPerf.polyBudgetTerrain;
            drawn = gNDSPerf.polysDrawnTerrain;
            break;
        case 1: // objects
            budget = gNDSPerf.polyBudgetObjects;
            drawn = gNDSPerf.polysDrawnObjects;
            break;
        case 2: // effects
            budget = gNDSPerf.polyBudgetEffects;
            drawn = gNDSPerf.polysDrawnEffects;
            break;
        default:
            return true;
    }

    if (drawn + polyCount > budget)
    {
        gNDSPerf.objectsCulledByBudget++;
        gNDSPerf.budgetExceeded = true;
        return false;
    }

    // Also check global total
    if (gNDSPerf.polysThisFrame + polyCount > NDS_MAX_POLYS_PER_FRAME)
    {
        gNDSPerf.objectsCulledByBudget++;
        gNDSPerf.budgetExceeded = true;
        return false;
    }

    return true;
#else
    (void)polyCount;
    (void)budgetCategory;
    return true;    // no budget limit on non-NDS platforms
#endif
}


/****************** NDS ADD DRAWN POLYS ************************/

void NDS_AddDrawnPolys(int polyCount, int budgetCategory)
{
    gNDSPerf.polysThisFrame += polyCount;

    switch (budgetCategory)
    {
        case 0: gNDSPerf.polysDrawnTerrain += polyCount; break;
        case 1: gNDSPerf.polysDrawnObjects += polyCount; break;
        case 2: gNDSPerf.polysDrawnEffects += polyCount; break;
    }
}


/****************** NDS IS BUDGET EXCEEDED ************************/

Boolean NDS_IsBudgetExceeded(void)
{
    return gNDSPerf.budgetExceeded;
}
