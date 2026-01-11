#ifndef UTILS_H_
#define UTILS_H_

#include "types.h"

/**
 * @brief Clamp mouse X coordinate to stay inside the grid bounds.
 * @param grid_config Grid configuration to use for bounds.
 * @return Clamped X coordinate in pixels.
 */
int safe_mouse_x(GridConfig * grid_config);

/**
 * @brief Clamp mouse Y coordinate to stay inside the grid bounds.
 * @param grid_config Grid configuration to use for bounds.
 * @return Clamped Y coordinate in pixels.
 */
int safe_mouse_y(GridConfig * grid_config);

/**
 * @brief Clear movement/attack range flags for all cells in the map.
 * @param cell_arr Pointer to the first cell in the map array.
 * @param grid Grid configuration describing dimensions.
 */
void cell_flag_flush(Point *cell_arr, GridConfig *grid);

/**
 * @brief Allocate and initialize a new GridConfig.
 * @param g_off_x X offset of the grid in pixels.
 * @param g_off_y Y offset of the grid in pixels.
 * @param g_cell_size Cell size in pixels.
 * @param max_cell_x Number of cells horizontally.
 * @param max_cell_y Number of cells vertically.
 * @return Newly allocated GridConfig pointer.
 */
GridConfig *grid_init(int g_off_x, int g_off_y, int g_cell_size,
                      int max_cell_x, int max_cell_y);

#endif
