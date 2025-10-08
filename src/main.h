#ifndef MAIN_H_
#define MAIN_H_

#define GRID_CELL_SIZE 30
#define MAX_GRID_CELLS_X 30
#define MAX_GRID_CELLS_Y 21
#define DARK_TROOP_NUM 6
#define VENT_TROOP_NUM 6
#define GRID_OFFSET_X 40
#define GRID_OFFSET_Y 60

#include "types.h"

int safe_mouse_x(GridConfig * grid_config);
int safe_mouse_y(GridConfig * grid_config);
Point *mouseToCell(GridConfig * grid_config, Point *point_arr);
bool mouseInCell(GridConfig * grid_config, Point cell);
// bool = True or False
void cell_selection(GridConfig* grid, Point *cell_arr, Point *cell, Point **focused_cell);
void range_calc(GridConfig * grid, Point *cell_arr, Point *start_cell, int range, bool selection);
void spread_terrain(GridConfig * grid, Point *cell_arr, Point *start_cell, int range,
                    Terrain terrain);
Point *get_random_cell(GridConfig * grid, Point *cell_arr);
void generate_biome_cores(GridConfig * grid, Point *cell_arr, BiomeConfig config);
void generate_all_biomes(GridConfig * grid, Point *cell_arr, BiomeConfig *biome_configs,
                         int num_biomes, int layers);
void focused_cell_info(Point *selected_cell, GridConfig * grid_config);
void attack_range_calc(GridConfig * grid, Point *cell_arr, Point *start_cell, int range,
                       bool selection);
void cell_flag_flush(Point *cell_arr, GridConfig * grid);
Point *get_random_spawn_cell(GridConfig * grid, Point *cell_arr);
GridConfig * grid_init( int g_off_x, int g_off_y, int g_cell_size,
                  int max_cell_x, int max_cell_y);
#endif