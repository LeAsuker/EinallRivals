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

  // Darkus faction (player)
  factions[DARKUS].has_turn = true;
  factions[DARKUS].playable = true;
  factions[DARKUS].prim_color = PURPLE;
  factions[DARKUS].sec_color = DARKGRAY;
  strcpy(factions[DARKUS].name, "Darkus");
  factions[DARKUS].characters = NULL;
  factions[DARKUS].character_count = 0;

  // Ventus faction (Player)
  factions[VENTUS].has_turn = false;
  factions[VENTUS].playable = true;
  factions[VENTUS].prim_color = GREEN;
  factions[VENTUS].sec_color = WHITE;
  strcpy(factions[VENTUS].name, "Ventus");
  factions[VENTUS].characters = NULL;
  factions[VENTUS].character_count = 0;

  // Gaia faction (AI)
  factions[GAIA].has_turn = false;
  // Gaia are neutrals and should not take turns
  factions[GAIA].playable = false;
  factions[GAIA].prim_color = BROWN;
  factions[GAIA].sec_color = BLACK;
  strcpy(factions[GAIA].name, "Gaia");
  factions[GAIA].characters = NULL;
  factions[GAIA].character_count = 0;

  return 3;
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
