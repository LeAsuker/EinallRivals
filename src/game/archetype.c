#include "game/archetype.h"
#include "game/class_data.h"

// ============================================================================
// Archetype Slot Definitions
// ============================================================================

static const ArchetypeSlotConfig ARCHETYPE_CONFIGS[ARCHETYPE_COUNT] = {
    [ARCHETYPE_WARRIOR] =
        {
            .slot_count = 4,
            .slots = {EQUIP_SLOT_ARMOR, EQUIP_SLOT_WEAPON_MELEE,
                      EQUIP_SLOT_TRINKET, EQUIP_SLOT_TRINKET},
        },
    [ARCHETYPE_WARG] = {
        .slot_count = 0,
        .slots = {EQUIP_SLOT_NONE, EQUIP_SLOT_NONE, EQUIP_SLOT_NONE,
                  EQUIP_SLOT_NONE},
    },
};

// ============================================================================
// Archetype Accessors
// ============================================================================

const ArchetypeSlotConfig *archetype_get_slot_config(Archetype archetype) {
  if (archetype < 0 || archetype >= ARCHETYPE_COUNT) {
    return NULL;
  }
  return &ARCHETYPE_CONFIGS[archetype];
}

// ============================================================================
// Archetype Base Class
// ============================================================================

const struct UnitClass *archetype_get_base_class(Archetype archetype) {
  switch (archetype) {
  case ARCHETYPE_WARRIOR:
    return class_get_militia();
  case ARCHETYPE_WARG:
    return class_get_warg();
  default:
    return NULL;
  }
}
