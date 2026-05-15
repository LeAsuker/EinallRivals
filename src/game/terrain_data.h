#ifndef TERRAIN_DATA_H_
#define TERRAIN_DATA_H_

#include "game/terrain.h"

// ============================================================================
// Terrain Metadata (static data definitions)
// ============================================================================

/**
 * @brief Static metadata for a terrain type.
 *
 * Contains everything needed to initialize a Terrain entry except the
 * runtime-loaded Texture2D and the resolved deep_version pointer.
 */
typedef struct {
  int id;               /**< Terrain identifier */
  Color color;          /**< Fallback color used when no sprite present */
  bool passable;        /**< Whether units can enter this terrain tile */
  int deep_version;     /**< TerrainType index for deep variant, or -1 for NULL */
  const char *name;     /**< Short terrain name */
  const char *texture_path; /**< Resource path for terrain sprite (NULL if none) */
} TerrainMetadata;

// Predefined terrain metadata (statically allocated, shared across all init)
extern const TerrainMetadata TERRAIN_META_NONE;
extern const TerrainMetadata TERRAIN_META_PLAINS;
extern const TerrainMetadata TERRAIN_META_MOUNTAINS;
extern const TerrainMetadata TERRAIN_META_SEA;
extern const TerrainMetadata TERRAIN_META_ARCTIC;
extern const TerrainMetadata TERRAIN_META_FOREST;
extern const TerrainMetadata TERRAIN_META_DEEP_FOREST;
extern const TerrainMetadata TERRAIN_META_DEEP_SEA;
extern const TerrainMetadata TERRAIN_META_PLAYER_BASE;

/**
 * @brief Get a pointer to the TerrainMetadata for a given terrain type.
 * @param type TerrainType to look up.
 * @return Pointer to the corresponding TerrainMetadata (never NULL).
 */
const TerrainMetadata *terrain_data_get(TerrainType type);

#endif
