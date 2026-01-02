#include "core/utils.h"
#include <stdlib.h>

int safe_mouse_x(GridConfig * grid_config) {
  int mouse_pos = GetMouseX();
  if (mouse_pos >= grid_config->grid_offset_x + grid_config->max_grid_cells_x* grid_config->grid_cell_size) {
    return grid_config->grid_offset_x + grid_config->max_grid_cells_x * grid_config->grid_cell_size - 1;
  } else if (mouse_pos <= grid_config->grid_offset_x) {
    return grid_config->grid_offset_x + 1;
  }
  return mouse_pos;
}

int safe_mouse_y(GridConfig * grid_config) {
  int mouse_pos = GetMouseY();
  if (mouse_pos >= grid_config->grid_offset_y + grid_config->max_grid_cells_y * grid_config->grid_cell_size) {
    return grid_config->grid_offset_y + grid_config->max_grid_cells_y * grid_config->grid_cell_size - 1;
  } else if (mouse_pos <= grid_config->grid_offset_y) {
    return grid_config->grid_offset_y + 1;
  }
  return mouse_pos;
}

// Helper function moved from main.c — used elsewhere (e.g., input.c)
void cell_flag_flush(Point *cell_arr, GridConfig *grid) {
  for (int y = 0; y < grid->max_grid_cells_y; y++) {
    for (int x = 0; x < grid->max_grid_cells_x; x++) {
      cell_arr[x + y * grid->max_grid_cells_x].in_range = false;
      cell_arr[x + y * grid->max_grid_cells_x].in_attack_range = false;
    }
  }
}

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
