#ifndef SPAWNING_H_
#define SPAWNING_H_

#include "types.h"

/**
 * @brief Place all actors from a faction into random spawn cells in a map
 * corner.
 *
 * Uses map_get_random_corner_spawn_cell to find suitable spawn points within
 * the specified corner and area. Each actor in the faction's contiguous
 * `characters` array will be placed once; if insufficient spawn cells are
 * found, remaining actors will be left unplaced.
 *
 * @param mapArr Array of Point representing the map cells.
 * @param grid_config Grid configuration (dimensions and cell layout).
 * @param faction The faction whose characters will be spawned (must have
 *                `characters` and `character_count`).
 * @param corner Corner index (0..3) to spawn in.
 * @param area_size Radius/area to attempt spawning within the corner.
 * @param max_attempts Number of attempts per actor to find a free spawn cell.
 */
void spawning_place_faction_in_corner(Point *mapArr, GridConfig *grid_config,
                                      Faction *faction, int corner,
                                      int area_size, int max_attempts);

#endif
