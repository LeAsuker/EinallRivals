#include "game/faction_init.h"
#include "game/faction_data.h"
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
  if (max_factions < FACTION_COUNT) {
    return 0;
  }

  for (int i = 0; i < FACTION_COUNT; i++) {
    const FactionDef *def = faction_get_def((Factions)i);
    if (def == NULL) {
      continue;
    }
    factions[i].has_turn = def->has_turn;
    factions[i].playable = def->playable;
    factions[i].prim_color = def->prim_color;
    factions[i].sec_color = def->sec_color;
    strcpy(factions[i].name, def->name);
    factions[i].characters = NULL;
    factions[i].character_count = 0;
  }

  return FACTION_COUNT;
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
