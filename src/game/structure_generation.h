#ifndef STRUCTURE_GENERATION_H_
#define STRUCTURE_GENERATION_H_

#include "types.h"
#include "render/unit_sprites.h"
#include "render/structure_sprites.h"

// Places several Warg Lairs on the map. Returns the number of lairs placed.
int structure_generation_place_warg_lairs(Point *mapArr, GridConfig *grid_config,
                                          Terrain *terrains, int terrain_count,
                                          StructureSprites structure_sprites);

// Spawns Gaia wargs around already-placed Warg Lairs. Returns an allocated
// array of spawned wargs (or NULL) and writes the count into
// `out_warg_count`. Caller is responsible for freeing the returned array.
Actor *structure_generation_spawn_wargs_around_lairs(Point *mapArr, GridConfig *grid_config,
                                                     UnitSprites unit_sprites,
                                                     Faction *gaia_faction,
                                                     int *out_warg_count);

#endif
