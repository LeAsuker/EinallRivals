#ifndef FACTION_INIT_H_
#define FACTION_INIT_H_

#include "types.h"

typedef enum { DARKUS, VENTUS, GAIA } Factions;

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
 * @brief Free character arrays for every faction in `factions`.
 *
 * @param factions Array of factions.
 * @param faction_count Number of factions in the array.
 */
void factions_free_actors(Faction *factions, int faction_count);

#endif
