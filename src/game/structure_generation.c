#include "game/structure_generation.h"

#include <stdlib.h>
#include <string.h>

#include "game/structure.h"
#include "game/actor.h"
#include "game/map.h"
#include "game/terrain.h"

// Place lair structures only. Returns number of lairs placed.
int structure_generation_place_warg_lairs(Point *mapArr, GridConfig *grid_config,
                                          Terrain *terrains, int terrain_count,
                                          StructureSprites structure_sprites) {
    if (mapArr == NULL || grid_config == NULL || terrains == NULL) return 0;

    int num_lairs = (rand() % 3) + 4; // 4..6
    int lairs_placed = 0;
    int attempts = 0;

    while (lairs_placed < num_lairs && attempts < 1000) {
        attempts++;
        Point *candidate = map_get_random_cell(mapArr, grid_config);
        if (candidate == NULL) continue;
        if (candidate->terrain.id != terrains[TERRAIN_PLAINS].id) continue;
        if (candidate->occupant != NULL || candidate->structure != NULL) continue;

        // Place lair structure only
        Structure *lair = structure_create(structure_sprites.warg_lair, "Warg Lair", false, false);
        if (lair == NULL) continue;
        map_place_structure(mapArr, grid_config, candidate->x, candidate->y, lair);

        lairs_placed++;
    }

    return lairs_placed;
}

// Spawn wargs around already placed lairs. Returns allocated actor array and writes count.
Actor *structure_generation_spawn_wargs_around_lairs(Point *mapArr, GridConfig *grid_config,
                                                     UnitSprites unit_sprites,
                                                     Faction *gaia_faction,
                                                     int *out_warg_count) {
    if (mapArr == NULL || grid_config == NULL || gaia_faction == NULL || out_warg_count == NULL) {
        if (out_warg_count) *out_warg_count = 0;
        return NULL;
    }

    int offsets[8][2] = {{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}};

    // Count lairs to estimate maximum possible wargs
    int lair_count = 0;
    for (int y = 0; y < grid_config->max_grid_cells_y; y++) {
        for (int x = 0; x < grid_config->max_grid_cells_x; x++) {
            Point *p = map_get_cell(mapArr, grid_config, x, y);
            if (p && p->structure && strcmp(p->structure->name, "Warg Lair") == 0) {
                lair_count++;
            }
        }
    }

    int max_possible_wargs = lair_count * 3;
    Actor *gaia_wargs = NULL;
    int gaia_warg_count = 0;
    if (max_possible_wargs > 0) {
        gaia_wargs = malloc(sizeof(Actor) * max_possible_wargs);
        if (gaia_wargs == NULL) {
            *out_warg_count = 0;
            return NULL;
        }
    }

    // For each lair, try to spawn 2..3 wargs around it
    for (int y = 0; y < grid_config->max_grid_cells_y; y++) {
        for (int x = 0; x < grid_config->max_grid_cells_x; x++) {
            Point *p = map_get_cell(mapArr, grid_config, x, y);
            if (p == NULL || p->structure == NULL) continue;
            if (strcmp(p->structure->name, "Warg Lair") != 0) continue;

            ActorTemplate warg_template;
            actor_get_default_warg_template(&warg_template);
            int to_spawn = (rand() % 2) + 2; // 2..3
            int spawned = 0;
            for (int o = 0; o < 8 && spawned < to_spawn; o++) {
                int nx = p->x + offsets[o][0];
                int ny = p->y + offsets[o][1];
                if (!map_is_valid_coords(grid_config, nx, ny)) continue;
                Point *dest = map_get_cell(mapArr, grid_config, nx, ny);
                if (dest == NULL) continue;
                if (!map_can_unit_enter_cell(dest, NULL)) continue;

                actor_init_from_template(&gaia_wargs[gaia_warg_count], gaia_faction, unit_sprites.warg, &warg_template);
                dest->occupant = &gaia_wargs[gaia_warg_count];
                gaia_warg_count++;
                spawned++;
                if (gaia_warg_count >= max_possible_wargs) break;
            }
        }
    }

    *out_warg_count = gaia_warg_count;
    if (gaia_warg_count == 0) {
        if (gaia_wargs) {
            free(gaia_wargs);
            gaia_wargs = NULL;
        }
    }

    return gaia_wargs;
}
