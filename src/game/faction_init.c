#include "game/faction_init.h"
#include <string.h>
#include <stdlib.h>

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

void faction_free_characters(Faction *faction) {
    if (faction == NULL || faction->characters == NULL) return;
    
    free(faction->characters);
    faction->characters = NULL;
    faction->character_count = 0;
}

void factions_free_actors(Faction *factions, int faction_count) {
    if (factions == NULL || faction_count <= 0) return;
    
    for (int i = 0; i < faction_count; i++) {
        faction_free_characters(&factions[i]);
    }
}
