#include "game/structure_generation.h"

#include <stdlib.h>
#include <string.h>

#include "game/actions.h"
#include "game/actor.h"
#include "game/map.h"
#include "game/structure.h"
#include "game/structure_data.h"
#include "game/terrain.h"

/**
 * @brief Place several Warg Lairs on the map and return how many were placed.
 *
 * Scans for plains tiles and attempts to place 4..6 lairs at valid locations.
 */
int structure_generation_place_warg_lairs(Point *mapArr,
                                          GridConfig *grid_config,
                                          Terrain *terrains, int terrain_count,
                                          StructureSprites structure_sprites) {
  (void)terrain_count;
  if (mapArr == NULL || grid_config == NULL || terrains == NULL)
    return 0;

  int num_lairs = (rand() % 3) + 4; // 4..6
  int lairs_placed = 0;
  int attempts = 0;

  const StructureMetadata *meta = structure_data_get(STRUCTURE_WARG_LAIR);

  while (lairs_placed < num_lairs && attempts < 1000) {
    attempts++;
    Point *candidate = map_get_random_cell(mapArr, grid_config);
    if (candidate == NULL)
      continue;
    if (candidate->terrain.id != terrains[TERRAIN_PLAINS].id)
      continue;
    if (candidate->occupant != NULL || candidate->structure != NULL)
      continue;

    // Place lair structure only
    Structure *lair =
        structure_create(structure_sprites.warg_lair, meta->name,
                         meta->passable, meta->lootable);
    if (lair == NULL)
      continue;
    map_place_structure(mapArr, grid_config, candidate->x, candidate->y, lair);

    lairs_placed++;
  }

  return lairs_placed;
}

// Spawn wargs around already placed lairs. Returns allocated character array
// and writes count.
/**
 * @brief Spawn creatures (wargs) around previously placed Warg Lairs.
 *
 * Allocates and returns an array of Characters that were spawned, writing
 * the count into `out_warg_count`. Caller must free the returned array.
 */
Character *structure_generation_spawn_wargs_around_lairs(
    Point *mapArr, GridConfig *grid_config, UnitSprites unit_sprites,
    Faction *gaia_faction, int *out_warg_count,
    const ActionIcons *action_icons) {
  if (mapArr == NULL || grid_config == NULL || gaia_faction == NULL ||
      out_warg_count == NULL) {
    if (out_warg_count)
      *out_warg_count = 0;
    return NULL;
  }

  int offsets[8][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0},
                       {1, 0},   {-1, 1}, {0, 1},  {1, 1}};

  // Count lairs to estimate maximum possible wargs
  int lair_count = 0;
  for (int y = 0; y < grid_config->max_grid_cells_y; y++) {
    for (int x = 0; x < grid_config->max_grid_cells_x; x++) {
      Point *p = map_get_cell(mapArr, grid_config, x, y);
      if (p && p->structure && structure_is_type(p->structure, STRUCTURE_WARG_LAIR)) {
        lair_count++;
      }
    }
  }

  int max_possible_wargs = lair_count * 3;
  Character *gaia_wargs = NULL;
  int gaia_warg_count = 0;
  if (max_possible_wargs > 0) {
    gaia_wargs = malloc(sizeof(Character) * max_possible_wargs);
    if (gaia_wargs == NULL) {
      *out_warg_count = 0;
      return NULL;
    }
  }

  // For each lair, try to spawn 2..3 wargs around it
  for (int y = 0; y < grid_config->max_grid_cells_y; y++) {
    for (int x = 0; x < grid_config->max_grid_cells_x; x++) {
      Point *p = map_get_cell(mapArr, grid_config, x, y);
      if (p == NULL || p->structure == NULL)
        continue;
      if (!structure_is_type(p->structure, STRUCTURE_WARG_LAIR))
        continue;

      const UnitClass *warg_class = class_get_warg();
      int to_spawn = (rand() % 2) + 2; // 2..3
      int spawned = 0;
      for (int o = 0; o < 8 && spawned < to_spawn; o++) {
        int nx = p->x + offsets[o][0];
        int ny = p->y + offsets[o][1];
        if (!map_is_valid_coords(grid_config, nx, ny))
          continue;
        Point *dest = map_get_cell(mapArr, grid_config, nx, ny);
        if (dest == NULL)
          continue;
        if (!map_can_unit_enter_cell(dest, NULL))
          continue;

        character_init_from_class(&gaia_wargs[gaia_warg_count], gaia_faction,
                                  unit_sprites.warg, warg_class,
                                  ARCHETYPE_WARG, action_icons);
        dest->occupant = &gaia_wargs[gaia_warg_count];
        gaia_warg_count++;
        spawned++;
        if (gaia_warg_count >= max_possible_wargs)
          break;
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

/**
 * @brief Place abandoned huts at passable, unoccupied locations on the map.
 *
 * Returns the number of huts successfully placed.
 */
int structure_generation_place_abandoned_huts(
    Point *mapArr, GridConfig *grid_config, Terrain *terrains,
    int terrain_count, StructureSprites structure_sprites) {
  (void)terrain_count;
  if (mapArr == NULL || grid_config == NULL || terrains == NULL)
    return 0;

  int num_huts = (rand() % 5) + 6; // 6..10
  int huts_placed = 0;
  int attempts = 0;

  const StructureMetadata *meta = structure_data_get(STRUCTURE_ABANDONED_HUT);

  while (huts_placed < num_huts && attempts < 1000) {
    attempts++;
    Point *candidate = map_get_random_cell(mapArr, grid_config);
    if (candidate == NULL)
      continue;
    if (candidate->terrain.passable != true)
      continue;
    if (candidate->occupant != NULL || candidate->structure != NULL)
      continue;

    // Place abandoned hut structure only
    Structure *hut = structure_create(structure_sprites.abandoned_hut,
                                      meta->name, meta->passable,
                                      meta->lootable);
    if (hut == NULL)
      continue;
    map_place_structure(mapArr, grid_config, candidate->x, candidate->y, hut);

    huts_placed++;
  }

  return huts_placed;
}
