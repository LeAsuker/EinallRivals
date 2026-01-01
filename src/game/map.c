#include "game/map.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Forward declarations for internal helper functions
static void calculate_range_recursive(GridConfig *grid_config, Point *map,
                                      Point *current_cell, int remaining_range,
                                      bool enable, bool is_attack_range);

// ============================================================================
// Map Creation and Initialization
// ============================================================================

Point *map_create(GridConfig *grid_config, Terrain default_terrain) {
    int total_cells = grid_config->max_grid_cells_x * grid_config->max_grid_cells_y;
    Point *map = malloc(sizeof(Point) * total_cells);
    
    if (map == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for map\n");
        return NULL;
    }
    
    map_init_cells(map, grid_config, default_terrain);
    return map;
}

void map_free(Point *map) {
    if (map != NULL) {
        free(map);
    }
}

void map_init_cells(Point *map, GridConfig *grid_config, Terrain default_terrain) {
    for (int y = 0; y < grid_config->max_grid_cells_y; y++) {
        for (int x = 0; x < grid_config->max_grid_cells_x; x++) {
            int index = x + y * grid_config->max_grid_cells_x;
            map[index].x = x;
            map[index].y = y;
            map[index].occupant = NULL;
            map[index].in_range = false;
            map[index].in_attack_range = false;
            map[index].terrain = default_terrain;
        }
    }
}

// ============================================================================
// Cell Access and Utilities
// ============================================================================

Point *map_get_cell(Point *map, GridConfig *grid_config, int x, int y) {
    if (!map_is_valid_coords(grid_config, x, y)) {
        return NULL;
    }
    return map + x + y * grid_config->max_grid_cells_x;
}

Point *map_get_random_cell(Point *map, GridConfig *grid_config) {
    int rand_x = rand() % grid_config->max_grid_cells_x;
    int rand_y = rand() % grid_config->max_grid_cells_y;
    return map + rand_x + rand_y * grid_config->max_grid_cells_x;
}

Point *map_get_random_spawn_cell(Point *map, GridConfig *grid_config) {
    Point *cell = map_get_random_cell(map, grid_config);
    int max_attempts = 1000; // Prevent infinite loop
    int attempts = 0;
    
    // Keep trying until we find a valid spawn location
    while ((cell->terrain.id == 2 || cell->occupant != NULL) && attempts < max_attempts) {
        cell = map_get_random_cell(map, grid_config);
        attempts++;
    }
    
    if (attempts >= max_attempts) {
        fprintf(stderr, "Warning: Could not find valid spawn cell after %d attempts\n", max_attempts);
    }
    
    return cell;
}

bool map_is_valid_coords(GridConfig *grid_config, int x, int y) {
    return (x >= 0 && x < grid_config->max_grid_cells_x &&
            y >= 0 && y < grid_config->max_grid_cells_y);
}

Point * map_get_random_corner_cell(Point * mapArr, GridConfig * grid_config, int corner, int area_size) {
    // 0: top left and then like the clock 
    // +1 so its at least one and in bounds
    int x_offset = rand() % area_size + 1;
    int y_offset = rand() % area_size + 1;

    int x_corner, y_corner;

    // top left
    if ( corner == 0 ) {
        x_corner = 0;
        y_corner = 0;
        return map_get_cell(mapArr, grid_config, x_corner + x_offset, y_corner + y_offset);
    }
    // top right
    if ( corner == 1 ) {
        x_corner = grid_config->max_grid_cells_x;
        y_corner = 0;
        return map_get_cell(mapArr, grid_config, x_corner - x_offset, y_corner + y_offset);
    }
    // bottom right
    if ( corner == 2 ) {
        x_corner = grid_config->max_grid_cells_x;
        y_corner = grid_config->max_grid_cells_y;
        return map_get_cell(mapArr, grid_config, x_corner - x_offset, y_corner - y_offset);
    }
    // bottom left
    if ( corner == 3 ) {
        x_corner = 0;
        y_corner = grid_config->max_grid_cells_y;
        return map_get_cell(mapArr, grid_config, x_corner + x_offset, y_corner - y_offset);
    }

    assert(false);     
    // dummy for now
}

Point * map_get_random_corner_spawn_cell(Point* mapArr, GridConfig* grid_config, int corner, int area_size, int max_attempts) {
    // First: try a number of random attempts within the corner area
    for (int i = 0; i < max_attempts; i++) {
        Point *cell = map_get_random_corner_cell(mapArr, grid_config, corner, area_size);
        if (cell != NULL && !map_is_cell_occupied(cell) && map_is_terrain_passable(cell->terrain)) {
            return cell;
        }
    }

    // Second: deterministic scan of the corner area (guarantee we check every cell in area)
    int max_x = grid_config->max_grid_cells_x - 1;
    int max_y = grid_config->max_grid_cells_y - 1;
    int start_x, start_y, end_x, end_y;

    if (corner == 0) {
        start_x = 0; start_y = 0;
        end_x = (area_size < max_x) ? area_size : max_x;
        end_y = (area_size < max_y) ? area_size : max_y;
    } else if (corner == 1) {
        end_x = max_x; start_y = 0;
        start_x = (max_x - area_size > 0) ? (max_x - area_size) : 0;
        end_y = (area_size < max_y) ? area_size : max_y;
    } else if (corner == 2) {
        end_x = max_x; end_y = max_y;
        start_x = (max_x - area_size > 0) ? (max_x - area_size) : 0;
        start_y = (max_y - area_size > 0) ? (max_y - area_size) : 0;
    } else { // corner == 3
        start_x = 0; end_y = max_y;
        end_x = (area_size < max_x) ? area_size : max_x;
        start_y = (max_y - area_size > 0) ? (max_y - area_size) : 0;
    }

    for (int y = start_y; y <= end_y; y++) {
        for (int x = start_x; x <= end_x; x++) {
            Point *cell = map_get_cell(mapArr, grid_config, x, y);
            if (cell != NULL && !map_is_cell_occupied(cell) && map_is_terrain_passable(cell->terrain)) {
                return cell;
            }
        }
    }

    // Third: fallback to scanning the entire map for any valid spawn cell
    for (int y = 0; y <= max_y; y++) {
        for (int x = 0; x <= max_x; x++) {
            Point *cell = map_get_cell(mapArr, grid_config, x, y);
            if (cell != NULL && !map_is_cell_occupied(cell) && map_is_terrain_passable(cell->terrain)) {
                return cell;
            }
        }
    }

    // If we still didn't find anything (extremely unlikely), return (0,0)
    fprintf(stderr, "Warning: No valid spawn cell found; returning (0,0)\n");
    return map_get_cell(mapArr, grid_config, 0, 0);
}

bool map_all_8_neighs_terrain(Point * mapArr, GridConfig* grid, Point * cell, Terrain terrain) {
    int cell_x = cell->x;
    int cell_y = cell->y;
    Terrain cell_terrain = cell->terrain;

    // wtf was this why couldnt i declare it with a star
    int offset[3] = {-1, 0, 1};
    for (int l = 0; l < 3; l++) {
        for (int k = 0; k < 3; k++) {
            Point * neigh = map_get_cell(mapArr, grid, cell_x + offset[k], cell_y + offset[l]);
            if (neigh != NULL && neigh->terrain.id != cell_terrain.id && neigh->terrain.id != cell->terrain.deep_version->id) {
                return false;
            }
            // deep version is none
            if (cell->terrain.deep_version->id == -1) {
                return false;
            }
        }
    }

    return true;
}

// ============================================================================
// Range Calculations
// ============================================================================

void map_calculate_movement_range(GridConfig *grid_config, Point *map,
                                   Point *start_cell, int range, bool enable) {
    // Don't calculate range if starting from impassable terrain (e.g., sea)
    if (!map_is_terrain_passable(start_cell->terrain)) {
        return;
    }
    
    calculate_range_recursive(grid_config, map, start_cell, range, enable, false);
}

void map_calculate_attack_range(GridConfig *grid_config, Point *map,
                                Point *start_cell, int range, bool enable) {
    calculate_range_recursive(grid_config, map, start_cell, range, enable, true);
}

void map_clear_range_flags(Point *map, GridConfig *grid_config) {
    int total_cells = grid_config->max_grid_cells_x * grid_config->max_grid_cells_y;
    
    for (int i = 0; i < total_cells; i++) {
        map[i].in_range = false;
        map[i].in_attack_range = false;
    }
}

// ============================================================================
// Terrain Generation
// ============================================================================

void map_spread_terrain(GridConfig *grid_config, Point *map,
                       Point *start_cell, int range, Terrain terrain) {
    // Set current cell's terrain
    start_cell->terrain = terrain;
    
    // Base case: stop spreading
    if (range == 0) {
        return;
    }
    
    int x = start_cell->x;
    int y = start_cell->y;
    
    // Spread to adjacent cells (up, down, left, right)
    Point *neighbor;
    
    // Up
    if ((neighbor = map_get_cell(map, grid_config, x, y - 1)) != NULL) {
        map_spread_terrain(grid_config, map, neighbor, range - 1, terrain);
    }
    
    // Down
    if ((neighbor = map_get_cell(map, grid_config, x, y + 1)) != NULL) {
        map_spread_terrain(grid_config, map, neighbor, range - 1, terrain);
    }
    
    // Left
    if ((neighbor = map_get_cell(map, grid_config, x - 1, y)) != NULL) {
        map_spread_terrain(grid_config, map, neighbor, range - 1, terrain);
    }
    
    // Right
    if ((neighbor = map_get_cell(map, grid_config, x + 1, y)) != NULL) {
        map_spread_terrain(grid_config, map, neighbor, range - 1, terrain);
    }
}

void map_generate_biome_cores(GridConfig *grid_config, Point *map,
                              BiomeConfig config) {
    // Generate random number of cores (0 to max_cores)
    int num_cores = rand() % (config.max_cores + 1);
    
    for (int i = 0; i < num_cores; i++) {
        Point *core = map_get_random_cell(map, grid_config);
        int range = (rand() % config.max_range) + 1;
        map_spread_terrain(grid_config, map, core, range, config.terrain);
    }
}

void map_generate_all_biomes(GridConfig *grid_config, Point *map,
                             BiomeConfig *biome_configs, int num_biomes, int layers) {
    for (int layer = 0; layer < layers; layer++) {
        for (int i = 0; i < num_biomes; i++) {
            map_generate_biome_cores(grid_config, map, biome_configs[i]);
        }
    }

}

void map_generate_deep_ter(Point *map, GridConfig * grid) {
    for (int i = 0; i < grid->max_grid_cells_x*grid->max_grid_cells_y; i++) {
        Point* cell = map + i;
        if (map_all_8_neighs_terrain(map, grid, cell, cell->terrain)) {
        cell->terrain = *(cell->terrain.deep_version);
        }
    }
    return;
}

void map_generate_bases (Point* map, GridConfig* grid) {

}

// ============================================================================
// Terrain Queries
// ============================================================================

bool map_is_terrain_passable(Terrain terrain) {
    // Sea (id == 2) is not passable for ground units
    // You can expand this with more terrain rules
    return terrain.passable;
}

bool map_is_cell_occupied(Point *cell) {
    return cell->occupant != NULL;
}

bool map_can_unit_enter_cell(Point *cell, Actor *unit) {
    // Check if cell is already occupied
    if (map_is_cell_occupied(cell)) {
        return false;
    }
    
    // Check if terrain is passable
    // (In the future, you might check unit->can_fly or unit->movement_type here)
    if (!map_is_terrain_passable(cell->terrain)) {
        return false;
    }
    
    return true;
}


// ============================================================================
// Internal Helper Functions
// ============================================================================

static void calculate_range_recursive(GridConfig *grid_config, Point *map,
                                      Point *current_cell, int remaining_range,
                                      bool enable, bool is_attack_range) {

    // cant go through sea, later integrate traversability
    // can still go into sea hmmm
    if (!(is_attack_range) && !map_is_terrain_passable(current_cell->terrain)) {
        return;
    }

    // Set the appropriate flag for this cell
    if (is_attack_range) {
        current_cell->in_attack_range = enable;
    } else {
        current_cell->in_range = enable;
    }
    
    // Base case: no more range to spread
    if (remaining_range == 0) {
        return;
    }
    
    int x = current_cell->x;
    int y = current_cell->y;
    
    // Recursively calculate range for all adjacent cells
    Point *neighbor;
    
    // Up
    if ((neighbor = map_get_cell(map, grid_config, x, y - 1)) != NULL) {
        calculate_range_recursive(grid_config, map, neighbor, 
                                 remaining_range - 1, enable, is_attack_range);
    }
    
    // Down
    if ((neighbor = map_get_cell(map, grid_config, x, y + 1)) != NULL) {
        calculate_range_recursive(grid_config, map, neighbor, 
                                 remaining_range - 1, enable, is_attack_range);
    }
    
    // Left
    if ((neighbor = map_get_cell(map, grid_config, x - 1, y)) != NULL) {
        calculate_range_recursive(grid_config, map, neighbor, 
                                 remaining_range - 1, enable, is_attack_range);
    }
    
    // Right
    if ((neighbor = map_get_cell(map, grid_config, x + 1, y)) != NULL) {
        calculate_range_recursive(grid_config, map, neighbor, 
                                 remaining_range - 1, enable, is_attack_range);
    }
}
