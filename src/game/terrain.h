#ifndef TERRAIN_H_
#define TERRAIN_H_

#include "types.h"
#include "raylib.h"

// Terrain indices for array access
typedef enum {
    TERRAIN_NONE = 0,
    TERRAIN_PLAINS,
    TERRAIN_MOUNTAINS,
    TERRAIN_SEA,
    TERRAIN_ARCTIC,
    TERRAIN_FOREST,
    TERRAIN_DEEP_FOREST,
    TERRAIN_DEEP_SEA,
    TERRAIN_PLAYER_BASE,
    TERRAIN_COUNT
} TerrainType;

// Initialize all terrains at once
/**
 * @brief Load terrain textures and initialize Terrain structs.
 * @param terrains Preallocated array of Terrain with at least TERRAIN_COUNT elements.
 * @param cell_size Size in pixels to scale terrain sprites to.
 */
void terrain_init_all(Terrain *terrains, int cell_size);

// Cleanup all terrain textures
/**
 * @brief Unload textures for the provided terrain array.
 * @param terrains Array of terrains to unload.
 * @param count Number of terrain entries.
 */
void terrain_unload_all(Terrain *terrains, int count);

#endif
