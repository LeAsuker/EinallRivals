#include "core/utils.h"

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
