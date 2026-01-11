#include "game/spawning.h"

#include <stdlib.h>

#include "game/map.h"

void spawning_place_faction_in_corner(Point *mapArr, GridConfig *grid_config,
                                      Faction *faction, int corner,
                                      int area_size, int max_attempts) {
    if (mapArr == NULL || grid_config == NULL || faction == NULL) return;
    if (faction->characters == NULL || faction->character_count <= 0) return;

    for (int i = 0; i < faction->character_count; i++) {
        Point *spawn = map_get_random_corner_spawn_cell(mapArr, grid_config, corner, area_size, max_attempts);
        if (spawn == NULL) break;
        spawn->occupant = &faction->characters[i];
    }
}
