#include "game/terrain_data.h"

// ============================================================================
// Default Terrain Metadata Definitions
// ============================================================================

const TerrainMetadata TERRAIN_META_NONE = {
    .id = -1,
    .color = WHITE,
    .passable = false,
    .deep_version = -1,
    .name = "None",
    .texture_path = NULL};

const TerrainMetadata TERRAIN_META_PLAINS = {.id = 0,
                                             .color = GREEN,
                                             .passable = true,
                                             .deep_version = -1,
                                             .name = "Plains",
                                             .texture_path = "terrain/plains_"
                                                            "ter.png"};

const TerrainMetadata TERRAIN_META_MOUNTAINS = {
    .id = 1,
    .color = LIGHTGRAY,
    .passable = false,
    .deep_version = -1,
    .name = "Mountains",
    .texture_path = "terrain/mountain_ter2.png"};

const TerrainMetadata TERRAIN_META_SEA = {.id = 2,
                                          .color = BLUE,
                                          .passable = false,
                                          .deep_version = 7, // TERRAIN_DEEP_SEA
                                          .name = "Sea",
                                          .texture_path = "terrain/sea_ter.png"};

const TerrainMetadata TERRAIN_META_ARCTIC = {
    .id = 3,
    .color = WHITE,
    .passable = true,
    .deep_version = 2, // TERRAIN_MOUNTAINS
    .name = "Hills",
    .texture_path = "terrain/arctic_ter.png"};

const TerrainMetadata TERRAIN_META_FOREST = {
    .id = 4,
    .color = DARKGREEN,
    .passable = true,
    .deep_version = 6, // TERRAIN_DEEP_FOREST
    .name = "Forest",
    .texture_path = "terrain/forest_ter2.png"};

const TerrainMetadata TERRAIN_META_DEEP_FOREST = {
    .id = 41,
    .color = BLACK,
    .passable = false,
    .deep_version = -1,
    .name = "Deep Forest",
    .texture_path = "terrain/deep_forest_ter.png"};

const TerrainMetadata TERRAIN_META_DEEP_SEA = {
    .id = 21,
    .color = DARKBLUE,
    .passable = false,
    .deep_version = -1,
    .name = "Deep Sea",
    .texture_path = "terrain/deep_sea_ter.png"};

const TerrainMetadata TERRAIN_META_PLAYER_BASE = {
    .id = 6,
    .color = ORANGE,
    .passable = true,
    .deep_version = -1,
    .name = "Base",
    .texture_path = "terrain/base_ter.png"};

// ============================================================================
// Terrain Metadata Getters
// ============================================================================

const TerrainMetadata *terrain_data_get(TerrainType type) {
  switch (type) {
  case TERRAIN_NONE:
    return &TERRAIN_META_NONE;
  case TERRAIN_PLAINS:
    return &TERRAIN_META_PLAINS;
  case TERRAIN_MOUNTAINS:
    return &TERRAIN_META_MOUNTAINS;
  case TERRAIN_SEA:
    return &TERRAIN_META_SEA;
  case TERRAIN_ARCTIC:
    return &TERRAIN_META_ARCTIC;
  case TERRAIN_FOREST:
    return &TERRAIN_META_FOREST;
  case TERRAIN_DEEP_FOREST:
    return &TERRAIN_META_DEEP_FOREST;
  case TERRAIN_DEEP_SEA:
    return &TERRAIN_META_DEEP_SEA;
  case TERRAIN_PLAYER_BASE:
    return &TERRAIN_META_PLAYER_BASE;
  default:
    return &TERRAIN_META_NONE;
  }
}
