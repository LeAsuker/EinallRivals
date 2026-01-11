#ifndef MAIN_H_
#define MAIN_H_

#define GRID_CELL_SIZE 40
#define MAX_GRID_CELLS_X 30
#define MAX_GRID_CELLS_Y 20
#define DARK_TROOP_NUM 6
#define VENT_TROOP_NUM 6
#define GRID_OFFSET_X 40
#define GRID_OFFSET_Y 60

#include "types.h"

/**
 * @file main.h
 * @brief Global configuration macros used by the `main` entry point and
 * other modules for grid sizing and offsets.
 */
// Historical/legacy prototypes were removed — individual modules publish
// their own APIs. This header now only contains configuration macros used
// by `main.c` and simple consumers.
#endif
