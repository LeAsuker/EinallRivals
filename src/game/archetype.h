#ifndef ARCHETYPE_H_
#define ARCHETYPE_H_

#include "types.h"

// ============================================================================
// Archetype Slot Configuration
// ============================================================================

/**
 * @brief Describes the equipment slot layout for a given archetype.
 *
 * Used during character creation to initialize the equipment array.
 */
typedef struct {
  int slot_count;
  EquipSlotType slots[MAX_EQUIP_SLOTS];
} ArchetypeSlotConfig;

/**
 * @brief Get the equipment slot configuration for an archetype.
 *
 * @param archetype The archetype to query.
 * @return Pointer to the static configuration, or NULL for invalid archetypes.
 */
const ArchetypeSlotConfig *archetype_get_slot_config(Archetype archetype);

/**
 * @brief Get the base UnitClass for an archetype.
 *
 * @param archetype The archetype to query.
 * @return Pointer to the base class template, or NULL for invalid archetypes.
 */
const struct UnitClass *archetype_get_base_class(Archetype archetype);

#endif
