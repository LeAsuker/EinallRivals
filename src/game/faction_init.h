#ifndef FACTION_INIT_H_
#define FACTION_INIT_H_

#include "game/faction_data.h"

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
int faction_init_default(Faction *factions, int max_factions);

/**
 * @brief Free a faction's characters array and reset its count.
 *
 * Safe to call with a NULL faction pointer or when `characters` is NULL.
 *
 * @param faction Faction whose character array should be freed.
 */
void faction_free_characters(Faction *faction);

/**
 * @brief Free characters for all factions in an array.
 *
 * Iterates and calls faction_free_characters for each entry.
 */
void factions_free_actors(Faction *factions, int faction_count);

#endif
