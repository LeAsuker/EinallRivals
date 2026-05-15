#ifndef STRUCTURE_DATA_H_
#define STRUCTURE_DATA_H_

#include "types.h"
#include <stdbool.h>

// ============================================================================
// Structure Type System
// ============================================================================

/** @brief Indices for predefined structure types. */
typedef enum {
  STRUCTURE_WARG_LAIR = 0,
  STRUCTURE_ABANDONED_HUT,
  STRUCTURE_COUNT
} StructureType;

/**
 * @brief Static metadata for a structure type.
 *
 * Contains everything needed to create a Structure except the runtime-loaded
 * Texture2D sprite.
 */
typedef struct {
  const char *name;     /**< Human-readable name */
  bool passable;        /**< Whether units can enter the tile */
  bool lootable;        /**< Whether the structure can be looted */
  const char *texture_path; /**< Resource path for structure sprite */
} StructureMetadata;

// Predefined structure metadata (statically allocated)
extern const StructureMetadata STRUCTURE_META_WARG_LAIR;
extern const StructureMetadata STRUCTURE_META_ABANDONED_HUT;

/**
 * @brief Get metadata for a structure type.
 * @param type StructureType to look up.
 * @return Pointer to the corresponding StructureMetadata (never NULL).
 */
const StructureMetadata *structure_data_get(StructureType type);

/**
 * @brief Check whether a structure matches the given type by name.
 * @param s Structure to test (NULL-safe, returns false).
 * @param type StructureType to compare against.
 * @return true if the structure's name matches the type's metadata name.
 */
bool structure_is_type(const Structure *s, StructureType type);

#endif
