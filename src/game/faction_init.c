#include "game/faction_init.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Populate a default set of factions (DARKUS, VENTUS, GAIA).
 *
 * Initializes faction properties (colors, names, playability) and clears
 * any character arrays. Returns the number of factions written.
 *
 * @param factions Preallocated array to initialize.
 * @param max_factions Capacity of the `factions` array.
 * @return Number of factions initialized (0 on insufficient capacity).
 */
int faction_init_default(Faction *factions, int max_factions) {
  if (max_factions < 3) {
    return 0;
  }

  static const struct {
    const char *name;
    Color prim_color;
    Color sec_color;
    bool playable;
    bool has_turn;
  } FACTION_DEFS[] = {
      {"Darkus", PURPLE, DARKGRAY, true, true},
      {"Ventus", GREEN, WHITE, true, false},
      {"Gaia", BROWN, BLACK, false, false},
  };
  int count = sizeof(FACTION_DEFS) / sizeof(FACTION_DEFS[0]);

  for (int i = 0; i < count; i++) {
    factions[i].has_turn = FACTION_DEFS[i].has_turn;
    factions[i].playable = FACTION_DEFS[i].playable;
    factions[i].prim_color = FACTION_DEFS[i].prim_color;
    factions[i].sec_color = FACTION_DEFS[i].sec_color;
    strcpy(factions[i].name, FACTION_DEFS[i].name);
    factions[i].characters = NULL;
    factions[i].character_count = 0;
  }

  return count;
}

/**
 * @brief Free a faction's characters array and reset its count.
 *
 * Safe to call with a NULL faction pointer or when `characters` is NULL.
 *
 * @param faction Faction whose character array should be freed.
 */
void faction_free_characters(Faction *faction) {
  if (faction == NULL || faction->characters == NULL)
    return;

  free(faction->characters);
  faction->characters = NULL;
  faction->character_count = 0;
}

/**
 * @brief Free characters for all factions in an array.
 *
 * Iterates and calls faction_free_characters for each entry.
 */
void factions_free_actors(Faction *factions, int faction_count) {
  if (factions == NULL || faction_count <= 0)
    return;

  for (int i = 0; i < faction_count; i++) {
    faction_free_characters(&factions[i]);
  }
}
