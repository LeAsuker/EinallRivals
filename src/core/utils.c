#include "core/utils.h"
#include <stdlib.h>

/**
 * @brief Clamp mouse X coordinate to remain inside the grid bounds.
 * @param grid_config Grid configuration used to compute bounds.
 * @return Clamped X coordinate (screen pixels).
 */
int safe_mouse_x(GridConfig * grid_config) {
  NULL_CHECK_RET(grid_config, 0);
  int mouse_pos = GetMouseX();
  if (mouse_pos >= grid_config->grid_offset_x + grid_config->max_grid_cells_x* grid_config->grid_cell_size) {
    return grid_config->grid_offset_x + grid_config->max_grid_cells_x * grid_config->grid_cell_size - 1;
  } else if (mouse_pos <= grid_config->grid_offset_x) {
    return grid_config->grid_offset_x + 1;
  }
  return mouse_pos;
}

/**
 * @brief Clamp mouse Y coordinate to remain inside the grid bounds.
 * @param grid_config Grid configuration used to compute bounds.
 * @return Clamped Y coordinate (screen pixels).
 */
int safe_mouse_y(GridConfig * grid_config) {
  NULL_CHECK_RET(grid_config, 0);
  int mouse_pos = GetMouseY();
  if (mouse_pos >= grid_config->grid_offset_y + grid_config->max_grid_cells_y * grid_config->grid_cell_size) {
    return grid_config->grid_offset_y + grid_config->max_grid_cells_y * grid_config->grid_cell_size - 1;
  } else if (mouse_pos <= grid_config->grid_offset_y) {
    return grid_config->grid_offset_y + 1;
  }
  return mouse_pos;
}

/**
 * @brief Clear all movement/attack range flags on a map cell array.
 * @param cell_arr Pointer to the first cell in the map array (width-major ordering).
 * @param grid Grid configuration used to iterate bounds.
 */
void cell_flag_flush(Point *cell_arr, GridConfig *grid) {
  NULL_CHECK_VOID(cell_arr);
  NULL_CHECK_VOID(grid);
  for (int y = 0; y < grid->max_grid_cells_y; y++) {
    for (int x = 0; x < grid->max_grid_cells_x; x++) {
      cell_arr[x + y * grid->max_grid_cells_x].in_range = false;
      cell_arr[x + y * grid->max_grid_cells_x].in_attack_range = false;
    }
  }
}

/**
 * @brief Allocate and initialize a GridConfig structure.
 * @param g_off_x Grid pixel X offset for top-left of the grid.
 * @param g_off_y Grid pixel Y offset for top-left of the grid.
 * @param g_cell_size Size of one grid cell in pixels.
 * @param max_cell_x Number of cells horizontally.
 * @param max_cell_y Number of cells vertically.
 * @return Newly allocated GridConfig (caller should free when done).
 */
GridConfig *grid_init(int g_off_x, int g_off_y, int g_cell_size,
                      int max_cell_x, int max_cell_y) {
  GridConfig *grid = malloc(sizeof(GridConfig));
  grid->grid_offset_x = g_off_x;
  grid->grid_offset_y = g_off_y;
  grid->grid_cell_size = g_cell_size;
  grid->max_grid_cells_x = max_cell_x;
  grid->max_grid_cells_y = max_cell_y;
  return grid;
}
