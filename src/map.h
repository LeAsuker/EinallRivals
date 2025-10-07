#ifndef MAP_H_
#define MAP_H_

#include "types.h"
#include <stdbool.h>

// Map management functions
Point *map_create(GridConfig *grid_config, Terrain default_terrain);
void map_free(Point *map);
void map_init_cells(Point *map, GridConfig *grid_config, Terrain default_terrain);

// Cell access and utilities
Point *map_get_cell(Point *map, GridConfig *grid_config, int x, int y);
Point *map_get_random_cell(Point *map, GridConfig *grid_config);
Point *map_get_random_spawn_cell(Point *map, GridConfig *grid_config);
bool map_is_valid_coords(GridConfig *grid_config, int x, int y);

// Range and pathfinding calculations
void map_calculate_movement_range(GridConfig *grid_config, Point *map, 
                                   Point *start_cell, int range, bool enable);
void map_calculate_attack_range(GridConfig *grid_config, Point *map, 
                                Point *start_cell, int range, bool enable);
void map_clear_range_flags(Point *map, GridConfig *grid_config);

// Terrain generation
void map_spread_terrain(GridConfig *grid_config, Point *map, 
                       Point *start_cell, int range, Terrain terrain);
void map_generate_biome_cores(GridConfig *grid_config, Point *map, 
                              BiomeConfig config);
void map_generate_all_biomes(GridConfig *grid_config, Point *map, 
                             BiomeConfig *biome_configs, int num_biomes, int layers);

// Terrain queries
bool map_is_terrain_passable(Terrain terrain);
bool map_is_cell_occupied(Point *cell);
bool map_can_unit_enter_cell(Point *cell, Actor *unit);

#endif