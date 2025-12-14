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
void terrain_init_all(Terrain *terrains, int cell_size);

// Cleanup all terrain textures
void terrain_unload_all(Terrain *terrains, int count);

#endif