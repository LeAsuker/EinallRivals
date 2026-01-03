#include "game/faction_init.h"
#include <string.h>

int faction_init_default(Faction *factions, int max_factions) {
    if (max_factions < 3) {
        return 0;
    }
    
    // Darkus faction (player)
    factions[0].has_turn = true;
    factions[0].playable = true;
    factions[0].prim_color = PURPLE;
    factions[0].sec_color = DARKGRAY;
    strcpy(factions[0].name, "Darkus");
    factions[0].actors = NULL;
    factions[0].actor_count = 0;
    
    // Ventus faction (Player)
    factions[1].has_turn = false;
    factions[1].playable = true;
    factions[1].prim_color = GREEN;
    factions[1].sec_color = WHITE;
    strcpy(factions[1].name, "Ventus");
    factions[1].actors = NULL;
    factions[1].actor_count = 0;
    
    // Gaia faction (AI)
    factions[2].has_turn = false;
    // Gaia are neutrals and should not take turns
    factions[2].playable = false;
    factions[2].prim_color = BROWN;
    factions[2].sec_color = BLACK;
    strcpy(factions[2].name, "Gaia");
    factions[2].actors = NULL;
    factions[2].actor_count = 0;
    
    return 3;
}
