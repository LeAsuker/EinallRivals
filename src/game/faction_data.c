#include "game/faction_data.h"

// ============================================================================
// Default Faction Definitions
// ============================================================================

const FactionDef FACTION_DEF_DARKUS = {
    .name = "Darkus",
    .prim_color = PURPLE,
    .sec_color = DARKGRAY,
    .playable = true,
    .has_turn = true};

const FactionDef FACTION_DEF_VENTUS = {
    .name = "Ventus",
    .prim_color = GREEN,
    .sec_color = WHITE,
    .playable = true,
    .has_turn = false};

const FactionDef FACTION_DEF_GAIA = {
    .name = "Gaia",
    .prim_color = BROWN,
    .sec_color = BLACK,
    .playable = false,
    .has_turn = false};

static const FactionDef *FACTION_DEFS[FACTION_COUNT] = {
    &FACTION_DEF_DARKUS,
    &FACTION_DEF_VENTUS,
    &FACTION_DEF_GAIA,
};

// ============================================================================
// Faction Definition Accessors
// ============================================================================

const FactionDef *faction_get_def(Factions faction) {
  if (faction < 0 || faction >= FACTION_COUNT) {
    return NULL;
  }
  return FACTION_DEFS[faction];
}

int faction_definition_count(void) { return FACTION_COUNT; }
