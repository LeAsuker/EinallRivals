#include "game/map.h"
#include "core/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
// Forward declarations for internal helper functions
static void calculate_range_recursive(GridConfig *grid_config, Point *map,
                                      Point *current_cell, int remaining_range,
                                      bool enable, bool is_attack_range);

// ============================================================================
// Map Creation and Initialization
// ============================================================================

/**
 * @brief Allocate and initialize a new map (array of Points).
 *
 * Allocates a contiguous array sized by the grid configuration and fills
 * each cell with `map_init_cells` using the given default terrain.
 *
 * @param grid_config Grid configuration describing dimensions.
 * @param default_terrain Terrain used to initialize every cell.
 * @return Pointer to allocated Point array or NULL on allocation failure.
 */
Point *map_create(GridConfig *grid_config, Terrain default_terrain) {
  int total_cells =
      grid_config->max_grid_cells_x * grid_config->max_grid_cells_y;
  Point *map = malloc(sizeof(Point) * total_cells);

  if (map == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for map\n");
    return NULL;
  }

  map_init_cells(map, grid_config, default_terrain);
  return map;
}

/**
 * @brief Free a map previously returned by map_create.
 *
 * Safe to call with NULL.
 *
 * @param map Pointer returned by map_create.
 */
void map_free(Point *map) {
  if (map != NULL) {
    free(map);
  }
}

/**
 * @brief Initialize every cell in a preallocated map array.
 *
 * Sets coordinates, clears occupant/flags/structure, and sets the terrain
 * for each cell to `default_terrain`.
 *
 * @param map Preallocated array of Points (size >= grid_config width*height).
 * @param grid_config Grid configuration used for dimensions.
 * @param default_terrain Terrain to assign to each cell.
 */
void map_init_cells(Point *map, GridConfig *grid_config,
                    Terrain default_terrain) {
  for (int y = 0; y < grid_config->max_grid_cells_y; y++) {
    for (int x = 0; x < grid_config->max_grid_cells_x; x++) {
      int index = x + y * grid_config->max_grid_cells_x;
      map[index].x = x;
      map[index].y = y;
      map[index].occupant = NULL;
      map[index].in_range = false;
      map[index].in_attack_range = false;
      map[index].structure = NULL;
      map[index].terrain = default_terrain;
    }
  }
}

// ============================================================================
// Cell Access and Utilities
// ============================================================================

/**
 * @brief Return a pointer to the cell at x,y or NULL if out of bounds.
 *
 * @param map Map array created with map_create.
 * @param grid_config Grid dimensions used for bounds checking.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return Pointer to the Point cell or NULL if coords are invalid.
 */
Point *map_get_cell(Point *map, GridConfig *grid_config, int x, int y) {
  if (!map_is_valid_coords(grid_config, x, y)) {
    return NULL;
  }
  return map + x + y * grid_config->max_grid_cells_x;
}

/**
 * @brief Return a random cell from the map.
 *
 * Uses rand() to select a coordinate within the grid bounds.
 *
 * @param map Map array.
 * @param grid_config Grid configuration for bounds.
 * @return Pointer to a randomly chosen cell.
 */
Point *map_get_random_cell(Point *map, GridConfig *grid_config) {
  int rand_x = rand() % grid_config->max_grid_cells_x;
  int rand_y = rand() % grid_config->max_grid_cells_y;
  return map + rand_x + rand_y * grid_config->max_grid_cells_x;
}

/**
 * @brief Return a random spawnable cell (not sea and not occupied).
 *
 * Retries up to a fixed number of attempts and logs a warning on failure.
 *
 * @param map Map array.
 * @param grid_config Grid configuration for bounds.
 * @return Pointer to a candidate spawn cell (may be invalid if search fails).
 */
Point *map_get_random_spawn_cell(Point *map, GridConfig *grid_config) {
  Point *cell = map_get_random_cell(map, grid_config);
  int max_attempts = 1000; // Prevent infinite loop
  int attempts = 0;

  // Keep trying until we find a valid spawn location
  while ((cell->terrain.id == 2 || cell->occupant != NULL) &&
         attempts < max_attempts) {
    cell = map_get_random_cell(map, grid_config);
    attempts++;
  }

  if (attempts >= max_attempts) {
    fprintf(stderr,
            "Warning: Could not find valid spawn cell after %d attempts\n",
            max_attempts);
  }

  return cell;
}

/**
 * @brief Check whether x,y coordinates fall inside grid bounds.
 *
 * @param grid_config Grid configuration providing max_grid_cells_x/y.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return true if coordinates are valid; false otherwise.
 */
bool map_is_valid_coords(const GridConfig *grid_config, int x, int y) {
  return (x >= 0 && x < grid_config->max_grid_cells_x && y >= 0 &&
          y < grid_config->max_grid_cells_y);
}

/**
 * @brief Pick a random cell near a chosen corner within `area_size` offset.
 *
 * corner: 0=top-left, 1=top-right, 2=bottom-right, 3=bottom-left.
 * Offsets are chosen randomly in [1, area_size].
 *
 * @param mapArr Map array.
 * @param grid_config Grid configuration for bounds.
 * @param corner Corner index (0..3) to choose which corner.
 * @param area_size Number of tiles inward from the corner to choose from.
 * @return Pointer to a randomly chosen corner cell (may be NULL if out of
 * bounds).
 */
Point *map_get_random_corner_cell(Point *mapArr, GridConfig *grid_config,
                                  int corner, int area_size) {
  // 0: top left and then like the clock
  // +1 so its at least one and in bounds
  int x_offset = rand() % area_size + 1;
  int y_offset = rand() % area_size + 1;

  int x_corner, y_corner;

  // top left
  if (corner == 0) {
    x_corner = 0;
    y_corner = 0;
    return map_get_cell(mapArr, grid_config, x_corner + x_offset,
                        y_corner + y_offset);
  }
  // top right
  if (corner == 1) {
    x_corner = grid_config->max_grid_cells_x;
    y_corner = 0;
    return map_get_cell(mapArr, grid_config, x_corner - x_offset,
                        y_corner + y_offset);
  }
  // bottom right
  if (corner == 2) {
    x_corner = grid_config->max_grid_cells_x;
    y_corner = grid_config->max_grid_cells_y;
    return map_get_cell(mapArr, grid_config, x_corner - x_offset,
                        y_corner - y_offset);
  }
  // bottom left
  if (corner == 3) {
    x_corner = 0;
    y_corner = grid_config->max_grid_cells_y;
    return map_get_cell(mapArr, grid_config, x_corner + x_offset,
                        y_corner - y_offset);
  }

  assert(false);
  // dummy for now
}

/**
 * @brief Try to find a free spawn cell inside a specified corner area.
 *
 * The function first attempts random picks within the corner area, then
 * deterministically scans the corner subregion, and finally falls back to
 * scanning the entire map for a candidate cell. Returns (0,0) on failure.
 *
 * @param mapArr Map array.
 * @param grid_config Grid configuration.
 * @param corner Corner index (0..3).
 * @param area_size Area radius to consider near the corner.
 * @param max_attempts Number of random attempts before deterministic scan.
 * @return Pointer to a suitable spawn cell.
 */
Point *map_get_random_corner_spawn_cell(Point *mapArr, GridConfig *grid_config,
                                        int corner, int area_size,
                                        int max_attempts) {
  // First: try a number of random attempts within the corner area
  for (int i = 0; i < max_attempts; i++) {
    Point *cell =
        map_get_random_corner_cell(mapArr, grid_config, corner, area_size);
    if (cell != NULL && !map_is_cell_occupied(cell) &&
        map_is_terrain_passable(cell->terrain)) {
      return cell;
    }
  }

  // Second: deterministic scan of the corner area (guarantee we check every
  // cell in area)
  int max_x = grid_config->max_grid_cells_x - 1;
  int max_y = grid_config->max_grid_cells_y - 1;
  int start_x, start_y, end_x, end_y;

  if (corner == 0) {
    start_x = 0;
    start_y = 0;
    end_x = (area_size < max_x) ? area_size : max_x;
    end_y = (area_size < max_y) ? area_size : max_y;
  } else if (corner == 1) {
    end_x = max_x;
    start_y = 0;
    start_x = (max_x - area_size > 0) ? (max_x - area_size) : 0;
    end_y = (area_size < max_y) ? area_size : max_y;
  } else if (corner == 2) {
    end_x = max_x;
    end_y = max_y;
    start_x = (max_x - area_size > 0) ? (max_x - area_size) : 0;
    start_y = (max_y - area_size > 0) ? (max_y - area_size) : 0;
  } else { // corner == 3
    start_x = 0;
    end_y = max_y;
    end_x = (area_size < max_x) ? area_size : max_x;
    start_y = (max_y - area_size > 0) ? (max_y - area_size) : 0;
  }

  for (int y = start_y; y <= end_y; y++) {
    for (int x = start_x; x <= end_x; x++) {
      Point *cell = map_get_cell(mapArr, grid_config, x, y);
      if (cell != NULL && !map_is_cell_occupied(cell) &&
          map_is_terrain_passable(cell->terrain)) {
        return cell;
      }
    }
  }

  // Third: fallback to scanning the entire map for any valid spawn cell
  for (int y = 0; y <= max_y; y++) {
    for (int x = 0; x <= max_x; x++) {
      Point *cell = map_get_cell(mapArr, grid_config, x, y);
      if (cell != NULL && !map_is_cell_occupied(cell) &&
          map_is_terrain_passable(cell->terrain)) {
        return cell;
      }
    }
  }

  // If we still didn't find anything (extremely unlikely), return (0,0)
  fprintf(stderr, "Warning: No valid spawn cell found; returning (0,0)\n");
  return map_get_cell(mapArr, grid_config, 0, 0);
}

/**
 * @brief Test whether all 8 neighboring cells share the same terrain (or its
 * deep variant).
 *
 * Returns false if any neighbor differs or if the cell's terrain has no
 * deep_version.
 *
 * @param mapArr Map array.
 * @param grid Grid configuration.
 * @param cell Cell to test.
 * @param terrain Terrain to compare against (unused; tests cell's
 * terrain/deep_version).
 * @return true if all neighbors match; false otherwise.
 */
bool map_all_8_neighs_terrain(Point *mapArr, GridConfig *grid, Point *cell,
                              Terrain terrain) {
  (void)terrain; // Parameter kept for API compatibility; cell's own terrain is used
  int cell_x = cell->x;
  int cell_y = cell->y;
  Terrain cell_terrain = cell->terrain;

  if (cell->terrain.deep_version == NULL || cell->terrain.deep_version->id == -1) {
    return false;
  }

  int offset[3] = {-1, 0, 1};
  for (int l = 0; l < 3; l++) {
    for (int k = 0; k < 3; k++) {
      if (k == 1 && l == 1)
        continue; // Skip the center cell itself
      Point *neigh =
          map_get_cell(mapArr, grid, cell_x + offset[k], cell_y + offset[l]);
      if (neigh != NULL && neigh->terrain.id != cell_terrain.id &&
          neigh->terrain.id != cell->terrain.deep_version->id) {
        return false;
      }
    }
  }

  return true;
}

// ============================================================================
// Range Calculations
// ============================================================================

/**
 * @brief Compute movement range from `start_cell` and set `in_range` flags.
 *
 * This will not compute ranges when starting on impassable terrain. Occupied
 * neighbor tiles block movement; the function temporarily clears the
 * starting cell occupant to prevent self-blocking.
 *
 * @param grid_config Grid configuration for bounds.
 * @param map Map array.
 * @param start_cell Origin cell for range calculation.
 * @param range Number of movement points.
 * @param enable Whether to enable (true) or disable (false) the flags.
 */
void map_calculate_movement_range(GridConfig *grid_config, Point *map,
                                  Point *start_cell, int range, bool enable) {
  // Don't calculate range if starting from impassable terrain (e.g., sea)
  if (!map_is_terrain_passable(start_cell->terrain)) {
    return;
  }

  // The recursive range calculation treats occupied cells as blockers.
  // Temporarily clear the starting cell's occupant so the unit's own tile
  // doesn't block range generation.
  Character *saved_occupant = start_cell->occupant;
  start_cell->occupant = NULL;
  calculate_range_recursive(grid_config, map, start_cell, range, enable, false);
  start_cell->occupant = saved_occupant;
}

/**
 * @brief Compute attack range from `start_cell` and set `in_attack_range`
 * flags.
 *
 * Attack range calculation allows traversing occupied tiles and uses the
 * same recursive helper as movement range with `is_attack_range` set to true.
 *
 * @param grid_config Grid configuration.
 * @param map Map array.
 * @param start_cell Origin for attack range.
 * @param range Attack range in tiles.
 * @param enable Whether to enable or disable the flags.
 */
void map_calculate_attack_range(GridConfig *grid_config, Point *map,
                                Point *start_cell, int range, bool enable) {
  calculate_range_recursive(grid_config, map, start_cell, range, enable, true);
}

/**
 * @brief Clear all movement and attack range flags in the map.
 *
 * @param map Map array.
 * @param grid_config Grid configuration for dimensions.
 */
void map_clear_range_flags(Point *map, GridConfig *grid_config) {
  int total = grid_total_cells(grid_config);
  for (int i = 0; i < total; i++) {
    map[i].in_range = false;
    map[i].in_attack_range = false;
  }
}

/**
 * @brief Clear only movement range flags (in_range) across the map.
 *
 * @param map Map array.
 * @param grid_config Grid configuration.
 */
void map_clear_movement_range_flags(Point *map, GridConfig *grid_config) {
  int total = grid_total_cells(grid_config);
  for (int i = 0; i < total; i++) {
    map[i].in_range = false;
  }
}

/**
 * @brief Clear only the attack range flags (in_attack_range) across the map.
 *
 * @param map Map array.
 * @param grid_config Grid configuration.
 */
void map_clear_attack_range_flags(Point *map, GridConfig *grid_config) {
  int total = grid_total_cells(grid_config);
  for (int i = 0; i < total; i++) {
    map[i].in_attack_range = false;
  }
}
// ============================================================================
// Terrain Generation
// ============================================================================

/**
 * @brief Recursively spread `terrain` outward from a starting cell up to
 * `range` steps.
 *
 * Performs a flood-fill limited by `range`, assigning `terrain` to visited
 * cells. Uses cardinal neighbors only. Bounds-checked via map_get_cell.
 *
 * @param grid_config Grid configuration for bounds.
 * @param map Map array.
 * @param start_cell Starting cell for spread.
 * @param range Number of steps to spread (0 = assign only the start cell).
 * @param terrain Terrain to apply.
 */
void map_spread_terrain(GridConfig *grid_config, Point *map, Point *start_cell,
                        int range, Terrain terrain) {
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

/**
 * @brief Place random cores for a biome and spread terrain from them.
 *
 * Chooses 0..max_cores random locations and spreads the biome terrain from
 * each core a random amount up to `max_range`.
 *
 * @param grid_config Grid configuration.
 * @param map Map array.
 * @param config Biome configuration describing terrain/range.
 */
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

/**
 * @brief Generate biomes across the map using provided `biome_configs`.
 *
 * Repeats `layers` times to create layered biome distributions.
 *
 * @param grid_config Grid configuration.
 * @param map Map array.
 * @param biome_configs Array of BiomeConfig entries.
 * @param num_biomes Number of entries in biome_configs.
 * @param layers Number of passes/layers to apply.
 */
void map_generate_all_biomes(GridConfig *grid_config, Point *map,
                             BiomeConfig *biome_configs, int num_biomes,
                             int layers) {
  for (int layer = 0; layer < layers; layer++) {
    for (int i = 0; i < num_biomes; i++) {
      map_generate_biome_cores(grid_config, map, biome_configs[i]);
    }
  }
}

/**
 * @brief Convert fully surrounded terrain tiles to their deep variant.
 *
 * For each cell, if all 8 neighbors match the cell's terrain (or deep
 * variant), replace the cell terrain with its deep version.
 *
 * @param map Map array.
 * @param grid Grid configuration.
 */
void map_generate_deep_ter(Point *map, GridConfig *grid) {
  for (int i = 0; i < grid->max_grid_cells_x * grid->max_grid_cells_y; i++) {
    Point *cell = map + i;
    if (map_all_8_neighs_terrain(map, grid, cell, cell->terrain)) {
      cell->terrain = *(cell->terrain.deep_version);
    }
  }
  return;
}

/**
 * @brief Placeholder for base placement logic on the map.
 *
 * Currently unimplemented; intended to place player/AI bases or starting
 * structures using map placement utilities.
 *
 * @param map Map array.
 * @param grid Grid configuration.
 */
void map_generate_bases(Point *map, GridConfig *grid) {}

// ============================================================================
// Terrain Queries
// ============================================================================

/**
 * @brief Test whether a terrain is passable for ground movement.
 *
 * Currently returns terrain.passable; extend for special rules (e.g.,
 * flying units) as needed.
 *
 * @param terrain Terrain to test.
 * @return true if passable; false otherwise.
 */
bool map_is_terrain_passable(Terrain terrain) {
  // Sea (id == 2) is not passable for ground units
  // You can expand this with more terrain rules
  return terrain.passable;
}

/**
 * @brief Return whether a cell currently has an occupying character.
 *
 * @param cell Cell to test.
 * @return true if occupied; false otherwise.
 */
bool map_is_cell_occupied(Point *cell) { return cell->occupant != NULL; }

/**
 * @brief Determine whether `unit` may enter `cell`.
 *
 * Considers occupancy, terrain passability, and structure blocking. Does not
 * currently consider unit-specific movement types.
 *
 * @param cell Target cell to enter.
 * @param unit Character attempting to enter (unused currently, reserved for
 * extensions).
 * @return true if the unit may enter; false otherwise.
 */
bool map_can_unit_enter_cell(Point *cell, Character *unit) {
  // Check if cell is already occupied
  if (map_is_cell_occupied(cell)) {
    return false;
  }

  // Check if terrain is passable
  // (In the future, you might check unit->can_fly or unit->movement_type here)
  if (!map_is_terrain_passable(cell->terrain)) {
    return false;
  }

  // Check if a structure blocks entry
  if (cell->structure != NULL && !cell->structure->passable) {
    return false;
  }

  return true;
}

// ============================================================================
// Structure placement
// ============================================================================

/**
 * @brief Place a structure at (x,y) on the map.
 *
 * Returns false if the coordinates are invalid or allocation fails. Overwrites
 * any existing structure pointer at the location.
 *
 * @param map Map array.
 * @param grid_config Grid configuration.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param s Structure pointer to place.
 * @return true on success; false otherwise.
 */
bool map_place_structure(Point *map, GridConfig *grid_config, int x, int y,
                         Structure *s) {
  if (!map_is_valid_coords(grid_config, x, y))
    return false;
  Point *cell = map_get_cell(map, grid_config, x, y);
  if (!cell)
    return false;
  cell->structure = s;
  return true;
}

/**
 * @brief Remove and return the structure pointer at (x,y).
 *
 * Does not free the structure memory; caller is responsible for cleanup.
 * Returns NULL for invalid coordinates.
 *
 * @param map Map array.
 * @param grid_config Grid configuration.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return The previous structure pointer or NULL.
 */
Structure *map_remove_structure(Point *map, GridConfig *grid_config, int x,
                                int y) {
  if (!map_is_valid_coords(grid_config, x, y))
    return NULL;
  Point *cell = map_get_cell(map, grid_config, x, y);
  if (!cell)
    return NULL;
  Structure *old = cell->structure;
  cell->structure = NULL;
  return old;
}

/**
 * @brief Return the structure pointer at (x,y) or NULL.
 *
 * @param map Map array.
 * @param grid_config Grid configuration.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return Structure pointer at location or NULL.
 */
Structure *map_get_structure_at(Point *map, GridConfig *grid_config, int x,
                                int y) {
  if (!map_is_valid_coords(grid_config, x, y))
    return NULL;
  Point *cell = map_get_cell(map, grid_config, x, y);
  if (!cell)
    return NULL;
  return cell->structure;
}

// ============================================================================
// Internal Helper Functions
// ============================================================================

/**
 * @brief Recursive helper used to mark movement or attack ranges.
 *
 * Traverses cardinal neighbors using remaining_range as depth. When
 * `is_attack_range` is true traversal ignores occupancy and structures.
 * The `enable` parameter sets or clears the corresponding flags on each
 * visited cell.
 */
static void calculate_range_recursive(GridConfig *grid_config, Point *map,
                                      Point *current_cell, int remaining_range,
                                      bool enable, bool is_attack_range) {

  // Movement range can't traverse impassable terrain, occupied cells, or
  // blocking structures.
  if (!is_attack_range) {
    if (!map_is_terrain_passable(current_cell->terrain)) {
      return;
    }
    if (current_cell->structure != NULL && !current_cell->structure->passable) {
      return;
    }
    if (map_is_cell_occupied(current_cell)) {
      return;
    }
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
    calculate_range_recursive(grid_config, map, neighbor, remaining_range - 1,
                              enable, is_attack_range);
  }

  // Down
  if ((neighbor = map_get_cell(map, grid_config, x, y + 1)) != NULL) {
    calculate_range_recursive(grid_config, map, neighbor, remaining_range - 1,
                              enable, is_attack_range);
  }

  // Left
  if ((neighbor = map_get_cell(map, grid_config, x - 1, y)) != NULL) {
    calculate_range_recursive(grid_config, map, neighbor, remaining_range - 1,
                              enable, is_attack_range);
  }

  // Right
  if ((neighbor = map_get_cell(map, grid_config, x + 1, y)) != NULL) {
    calculate_range_recursive(grid_config, map, neighbor, remaining_range - 1,
                              enable, is_attack_range);
  }
}
