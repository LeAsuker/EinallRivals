#include "game/spawning.h"

#include <stdlib.h>

#include "game/map.h"

/**
 * @brief Place each character from a faction into a random spawn cell in a 
 * specified corner of the map.
 *
 * Defensive: If any input pointer is NULL or the faction has no characters,
 * the function returns immediately.
 *
 * @param mapArr Array of map points.
 * @param grid_config Grid configuration describing map dimensions.
 * @param faction Faction containing characters to place.
 * @param corner Corner index (0..3) to select which corner to spawn in.
 * @param area_size Search radius within the corner.
 * @param max_attempts Number of attempts to find a spawn cell per character.
 */
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
