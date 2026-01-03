#ifndef SPAWNING_H_
#define SPAWNING_H_

#include "types.h"

// Place all actors owned by `faction` into spawn cells in the given corner.
// Uses `map_get_random_corner_spawn_cell` with the provided `area_size` and
// `max_attempts` parameters. Each actor in the faction's contiguous array
// will be placed once; if not enough free spawn cells are found some actors
// may remain unplaced.
void spawning_place_faction_in_corner(Point *mapArr, GridConfig *grid_config,
                                      Faction *faction, int corner,
                                      int area_size, int max_attempts);

#endif
