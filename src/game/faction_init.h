#ifndef FACTION_INIT_H_
#define FACTION_INIT_H_

#include "types.h"

typedef enum {
    DARKUS,
    VENTUS,
    GAIA
} Factions;

// Initialize default factions for the game
// Returns the number of factions initialized
int faction_init_default(Faction *factions, int max_factions);
void factions_free_actors(Faction *factions, int faction_count);

#endif
