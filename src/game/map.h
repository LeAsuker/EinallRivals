#ifndef MAP_H_
#define MAP_H_

/*
 * Module: Map Generation and Queries
 * Layer: Game Domain (src/game/)
 * Purpose: Map array allocation, biome spreading, terrain generation,
 * path/range calculations, cell queries, and structure placement.
 * Dependencies: types.h
 * Consumers: main.c, game/structure_generation.c, game/combat.c, input/input.c,
 * render/rendering.c
 * See AGENTS.md and src/CODEMAP.md for architecture context.
 */

#include "types.h"
#include <stdbool.h>

// Map management functions
/**
 * @brief Create a new map array initialized with the given default terrain.
 * @param grid_config Grid configuration describing map geometry.
 * @param default_terrain Terrain used to initialize cells.
 * @return Pointer to allocated Point array (width-major ordering).
 */
Point *map_create(GridConfig *grid_config, Terrain default_terrain);

/**
 * @brief Free a map array created by map_create.
 * @param map Map array to free.
 */
void map_free(Point *map);

/**
 * @brief Initialize cells of an existing map array to a default terrain.
 * @param map Map array to initialize.
 * @param grid_config Grid configuration describing array dimensions.
 * @param default_terrain Terrain to assign to each cell.
 */
void map_init_cells(Point *map, GridConfig *grid_config,
                    Terrain default_terrain);

// Cell access and utilities
/**
 * @brief Return pointer to the cell at (x,y) or NULL if out of bounds.
 */
Point *map_get_cell(Point *map, GridConfig *grid_config, int x, int y);

/**
 * @brief Return a random cell from the map (uniform selection).
 */
Point *map_get_random_cell(Point *map, GridConfig *grid_config);

/**
 * @brief Return a random spawnable cell from the map (suitable for unit
 * placement).
 */
Point *map_get_random_spawn_cell(Point *map, GridConfig *grid_config);

/**
 * @brief Check whether the coordinates (x,y) are inside the map bounds.
 */
bool map_is_valid_coords(const GridConfig *grid_config, int x, int y);

/**
 * @brief Get a random cell inside a corner area selection.
 */
Point *map_get_random_corner_cell(Point *mapArr, GridConfig *grid_config,
                                  int corner, int area_size);

/**
 * @brief Get a random spawn cell in a corner with a retry limit.
 */
Point *map_get_random_corner_spawn_cell(Point *mapArr, GridConfig *grid_config,
                                        int corner, int area_size,
                                        int max_attempts);

/**
 * @brief Return whether all 8 neighboring cells around `cell` are of a given
 * terrain.
 */
bool map_all_8_neighs_terrain(Point *mapArr, GridConfig *grid, Point *cell,
                              Terrain terrain);

// Range and pathfinding calculations
/**
 * @brief Calculate or clear movement range flags from a starting cell.
 */
void map_calculate_movement_range(GridConfig *grid_config, Point *map,
                                  Point *start_cell, int range, bool enable);

/**
 * @brief Calculate or clear attack range flags from a starting cell.
 */
void map_calculate_attack_range(GridConfig *grid_config, Point *map,
                                Point *start_cell, int range, bool enable);

/**
 * @brief Clear all range flags on the map.
 */
void map_clear_range_flags(Point *map, GridConfig *grid_config);

/**
 * @brief Clear only movement range flags on the map.
 */
void map_clear_movement_range_flags(Point *map, GridConfig *grid_config);

/**
 * @brief Clear only attack range flags on the map.
 */
void map_clear_attack_range_flags(Point *map, GridConfig *grid_config);

// Terrain generation
/**
 * @brief Spread a terrain type outwards from a start cell over a given range.
 */
void map_spread_terrain(GridConfig *grid_config, Point *map, Point *start_cell,
                        int range, Terrain terrain);

/**
 * @brief Generate several biome cores used as seeds for biome spreading.
 */
void map_generate_biome_cores(GridConfig *grid_config, Point *map,
                              BiomeConfig config);

/**
 * @brief Generate all biomes using supplied BiomeConfig array.
 */
void map_generate_all_biomes(GridConfig *grid_config, Point *map,
                             BiomeConfig *biome_configs, int num_biomes,
                             int layers);

/**
 * @brief Generate deeper terrain variants (e.g., convert to deep sea, caves).
 */
void map_generate_deep_ter(Point *map, GridConfig *grid);

// Terrain queries
/**
 * @brief Test whether a terrain type is passable for movement.
 */
bool map_is_terrain_passable(Terrain terrain);

/**
 * @brief Test whether a cell currently has an occupant.
 */
bool map_is_cell_occupied(Point *cell);

/**
 * @brief Check whether a unit may enter a specific cell (terrain+structure
 * checks).
 */
bool map_can_unit_enter_cell(Point *cell, Character *unit);

// Structure placement and queries
/**
 * @brief Place a structure at (x,y) if possible.
 */
bool map_place_structure(Point *map, GridConfig *grid_config, int x, int y,
                         Structure *s);

/**
 * @brief Remove and return a structure from (x,y), or NULL if none.
 */
Structure *map_remove_structure(Point *map, GridConfig *grid_config, int x,
                                int y);

/**
 * @brief Return pointer to a structure at (x,y) or NULL if none.
 */
Structure *map_get_structure_at(Point *map, GridConfig *grid_config, int x,
                                int y);
#endif
