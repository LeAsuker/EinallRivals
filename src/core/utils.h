#ifndef UTILS_H_
#define UTILS_H_

#include "types.h"

int safe_mouse_x(GridConfig * grid_config);
int safe_mouse_y(GridConfig * grid_config);
void cell_flag_flush(Point *cell_arr, GridConfig *grid);
GridConfig *grid_init(int g_off_x, int g_off_y, int g_cell_size,
					  int max_cell_x, int max_cell_y);

#endif
