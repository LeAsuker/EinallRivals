#include "game/structure_data.h"
#include <string.h>

// ============================================================================
// Default Structure Metadata Definitions
// ============================================================================

const StructureMetadata STRUCTURE_META_WARG_LAIR = {
    .name = "Warg Lair",
    .passable = true,
    .lootable = false,
    .texture_path = "structures/warg_lair.png"};

const StructureMetadata STRUCTURE_META_ABANDONED_HUT = {
    .name = "Abandoned Hut",
    .passable = false,
    .lootable = true,
    .texture_path = "structures/abandoned_hut.png"};

// ============================================================================
// Structure Metadata Getters
// ============================================================================

const StructureMetadata *structure_data_get(StructureType type) {
  switch (type) {
  case STRUCTURE_WARG_LAIR:
    return &STRUCTURE_META_WARG_LAIR;
  case STRUCTURE_ABANDONED_HUT:
    return &STRUCTURE_META_ABANDONED_HUT;
  default:
    return &STRUCTURE_META_WARG_LAIR;
  }
}

bool structure_is_type(const Structure *s, StructureType type) {
  if (s == NULL)
    return false;
  const StructureMetadata *meta = structure_data_get(type);
  return strcmp(s->name, meta->name) == 0;
}
