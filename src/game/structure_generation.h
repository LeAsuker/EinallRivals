#ifndef STRUCTURE_GENERATION_H_
#define STRUCTURE_GENERATION_H_

#include "render/structure_sprites.h"
#include "render/unit_sprites.h"
#include "types.h"

/**
 * @brief Place several Warg Lairs on the map and return how many were placed.
 *
 * Scans for plains terrain and attempts to place 4..6 lairs at valid locations.
 *
 * @param mapArr Map array.
 * @param grid_config Grid configuration.
 * @param terrains Array of terrains (indexed by enum).
 * @param terrain_count Number of terrains available.
 * @param structure_sprites Sprite set used for structure textures.
 * @return Number of lairs successfully placed.
 */
int structure_generation_place_warg_lairs(Point *mapArr,
                                          GridConfig *grid_config,
                                          Terrain *terrains, int terrain_count,
                                          StructureSprites structure_sprites);

/**
 * @brief Spawn creatures (wargs) around previously placed Warg Lairs.
 *
 * Allocates and returns an array of Characters that were spawned, writing
 * the count into `out_warg_count`. Caller must free the returned array.
 *
 * @param mapArr Map array.
 * @param grid_config Grid configuration.
 * @param unit_sprites Unit sprite set used for initializing characters.
 * @param gaia_faction Faction to assign spawned units to.
 * @param out_warg_count Output pointer to receive number of spawned wargs.
 * @return Allocated array of spawned Characters or NULL.
 */
Character *structure_generation_spawn_wargs_around_lairs(
    Point *mapArr, GridConfig *grid_config, UnitSprites unit_sprites,
    Faction *gaia_faction, int *out_warg_count);

/**
 * @brief Place abandoned huts at passable locations across the map.
 *
 * Attempts to place a small random number of huts at passable, unoccupied
 * locations. Returns the number actually placed.
 */
int structure_generation_place_abandoned_huts(
    Point *mapArr, GridConfig *grid_config, Terrain *terrains,
    int terrain_count, StructureSprites structure_sprites);
#endif
