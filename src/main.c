#include "raylib.h"

#include <stdio.h>
#include <stdlib.h> // Required for: calloc(), free()
#include <string.h> // Required for: memcpy(), strcmp()
#include <time.h>

#include "main.h"
#include "types.h"
#include "rendering.h"

int main(void) {
  Image sea_sprite = LoadImage("resources/sea_ter.png");
  Image mountains_sprite = LoadImage("resources/mountain_ter.png");
  Image plains_sprite = LoadImage("resources/plains_ter.png");
  Image arctic_sprite = LoadImage("resources/arctic_ter.png");
  Image forest_sprite = LoadImage("resources/forest_ter.png");

  Image v_militia_sprite = LoadImage("resources/ventus_militia.png");
  Image d_militia_sprite = LoadImage("resources/darkus_militia.png");

  ImageResize(&sea_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&mountains_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&plains_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&arctic_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&forest_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);

  ImageResize(&d_militia_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&v_militia_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);

  // Define terrain types
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 1280;
  const int screenHeight = 720;
  srand(time(NULL));

  GridConfig * grid_config = grid_init( GRID_OFFSET_X, GRID_OFFSET_Y, GRID_CELL_SIZE,
                                      MAX_GRID_CELLS_X, MAX_GRID_CELLS_Y);

  // initwindow creates opengl context, texture stuff needs to happen after it
  InitWindow(screenWidth, screenHeight, "WaterEmblemProto");
  Terrain Plains = {
      .id = 0, .color = GREEN, .sprite = LoadTextureFromImage(plains_sprite)};
  Terrain Mountains = {.id = 1,
                       .color = LIGHTGRAY,
                       .sprite = LoadTextureFromImage(mountains_sprite)};
  Terrain Sea = {
      .id = 2, .color = DARKBLUE, .sprite = LoadTextureFromImage(sea_sprite)};
  Terrain Arctic = {
      .id = 3, .color = WHITE, .sprite = LoadTextureFromImage(arctic_sprite)};
  Terrain Forest = {.id = 4,
                    .color = DARKGREEN,
                    .sprite = LoadTextureFromImage(forest_sprite)};

  strcpy(Plains.name, "Plains");
  strcpy(Mountains.name, "Mountains");
  strcpy(Sea.name, "Sea");
  strcpy(Arctic.name, "Arctic");
  strcpy(Forest.name, "Forest");

  // Configure each biome type
  BiomeConfig biome_configs[] = {
      {.terrain = Mountains, .max_cores = 3, .max_range = 3},
      {.terrain = Arctic, .max_cores = 3, .max_range = 3},
      {.terrain = Forest, .max_cores = 3, .max_range = 3},
      {.terrain = Sea, .max_cores = 2, .max_range = 5}};

  // map init
  Point *mapArr = malloc(sizeof(Point) * grid_config->max_grid_cells_x * grid_config->max_grid_cells_y);
  for (int yCoor = 0; yCoor < grid_config->max_grid_cells_y; yCoor++) {
    for (int xCoor = 0; xCoor < grid_config->max_grid_cells_x; xCoor++) {
      mapArr[xCoor + yCoor * grid_config->max_grid_cells_x].x = xCoor;
      mapArr[xCoor + yCoor * grid_config->max_grid_cells_x].y = yCoor;
      mapArr[xCoor + yCoor * grid_config->max_grid_cells_x].occupant = NULL;
      mapArr[xCoor + yCoor * grid_config->max_grid_cells_x].in_range = false;
      mapArr[xCoor + yCoor * grid_config->max_grid_cells_x].terrain = Plains;
    }
  }

  int num_biomes = sizeof(biome_configs) / sizeof(BiomeConfig);
  int layers = 7;
  generate_all_biomes(grid_config, mapArr, biome_configs, num_biomes, layers);

  RenderContext render_ctx;
  render_init(&render_ctx, grid_config);
  // init faction player
  Faction factions[] = {
      {.has_turn = true, .prim_color = PURPLE, .sec_color = DARKGRAY},
      {.has_turn = false, .prim_color = GREEN, .sec_color = WHITE},
      {.has_turn = false, .prim_color = BROWN, .sec_color = BLACK}};

  strcpy(factions[0].name, "Darkus");
  strcpy(factions[1].name, "Ventus");
  strcpy(factions[2].name, "Gaia");

  Actor *dark_troops = malloc(sizeof(Actor) * DARK_TROOP_NUM);
  Actor *vent_troops = malloc(sizeof(Actor) * VENT_TROOP_NUM);

  Texture2D d_militia_text = LoadTextureFromImage(d_militia_sprite);
  Texture2D v_militia_text = LoadTextureFromImage(v_militia_sprite);

  for (int i = 0; i < DARK_TROOP_NUM; i++) {
    actor_init(dark_troops + i, factions + 0, d_militia_text);
    Point *spawn = get_random_spawn_cell(grid_config, mapArr);
    spawn->occupant = dark_troops + i;
  }

  for (int i = 0; i < VENT_TROOP_NUM; i++) {
    actor_init(vent_troops + i, factions + 1, v_militia_text);
    Point *spawn = get_random_spawn_cell(grid_config, mapArr);
    spawn->occupant = vent_troops + i;
  }
  // Init current player state

  Faction *curr_faction = factions + 0;
  Point *focused_cell = NULL;

  SetTargetFPS(60);
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------
    // The XY coords are in the top left corner of the square
    // LMB
    Point *selected_cell = mouseToCell(grid_config, mapArr);
    if (IsMouseButtonPressed(0)) {
      // tells us which cell we have selected in the mapArr
      // had to do pointer stuff to point to the permanent object
      cell_selection(grid_config, mapArr, selected_cell, &focused_cell);
    }
    // RMB
    else if (IsMouseButtonPressed(1)) {
      // movement logic... supposedly segfault here
      if (focused_cell != NULL && selected_cell != NULL) {
        if ((focused_cell->occupant != NULL) && selected_cell->in_range &&
        selected_cell->occupant == NULL && focused_cell->occupant->can_move &&
        focused_cell->occupant->owner->has_turn) {
          // removed due to flushing function
          // range_calc(mapArr, last_player_position, player.movement, false);
          selected_cell->occupant = focused_cell->occupant;
          focused_cell->occupant = NULL;
          selected_cell->occupant->can_move = false;
          focused_cell = NULL;
        }
      }
      cell_flag_flush(mapArr, grid_config); // has to be after so player can move
    }

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    render_game(&render_ctx, mapArr, focused_cell, curr_faction->name);
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  free(mapArr);
  free(dark_troops);
  free(vent_troops);

  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

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

Point *mouseToCell(GridConfig * grid_config, Point *point_arr) {
  // will need to sanitize it so it cant ever go out of bounds, else undefined
  int x = (safe_mouse_x(grid_config) - grid_config->grid_offset_x) / grid_config->grid_cell_size;
  int y = (safe_mouse_y(grid_config) - grid_config->grid_offset_y) / grid_config->grid_cell_size;

  Point *cell_in_map = point_arr + grid_config->max_grid_cells_x * y + x;
  return cell_in_map;
}

// if mouse in given cell
bool mouseInCell(GridConfig * grid_config, Point cell) {
  int x_offset = grid_config->grid_offset_x;
  int y_offset = grid_config->grid_offset_y;
  int cell_size =  grid_config->grid_cell_size;

  if (safe_mouse_x(grid_config) <=
      x_offset + cell.x * cell_size + cell_size) {
    if (safe_mouse_y(grid_config) <=
        y_offset + cell.y * cell_size + cell_size) {
      if (safe_mouse_x(grid_config) >=
          x_offset + cell.x * cell_size) {
        if (safe_mouse_y(grid_config) >=
            y_offset + cell.y * cell_size) {
          return true;
        }
      }
    }
  }
  return false;
}
// could be issues here with occupant NULL checks...
// currently depends on in_range prev position flag
// removal
void cell_selection(GridConfig* grid, Point *cell_arr, Point *cell, Point **focused_cell) {
  // flushes the map of range indicator
  cell_flag_flush(cell_arr, grid );
  *focused_cell = cell;
  if (cell->occupant != NULL) {
    if (cell->occupant->owner->has_turn) {
      if (cell->occupant->can_move) {
        range_calc(grid, cell_arr, cell, cell->occupant->movement, true);
      }
      if (cell->occupant->can_act) {
        attack_range_calc(grid, cell_arr, cell, cell->occupant->range, true);
      }
    }
  }
  return;
}

void range_calc(GridConfig * grid, Point *cell_arr, Point *start_cell, int range, bool selection) {
  // quick fix, also removes targeting and affects flyers
  // comparison problem, will have to refactor this
  if (start_cell->terrain.id == 2) {
    return;
  }
  // base case
  start_cell->in_range = selection;
  if (range == 0) {
    return;
  }
  int x = start_cell->x;
  int y = start_cell->y;
  int x_limit = grid->max_grid_cells_x - 1;
  int y_limit = grid->max_grid_cells_y - 1;
  if (y != 0) {
    Point *cell_up = cell_arr + x + grid->max_grid_cells_x * (y - 1);
    range_calc(grid, cell_arr, cell_up, range - 1, selection);
  }
  if (y < y_limit) {

    Point *cell_down = cell_arr + x + grid->max_grid_cells_x * (y + 1);
    range_calc(grid, cell_arr, cell_down, range - 1, selection);
  }
  if (x != 0) {
    Point *cell_left = cell_arr + x - 1 + grid->max_grid_cells_x * (y);
    range_calc(grid, cell_arr, cell_left, range - 1, selection);
  }
  if (x < x_limit) {
    Point *cell_right = cell_arr + x + 1 + grid->max_grid_cells_x * (y);
    range_calc(grid, cell_arr, cell_right, range - 1, selection);
  }

  return;
}

// calcs attack range
void attack_range_calc(GridConfig * grid, Point *cell_arr, Point *start_cell, int range,
                       bool selection) {
  start_cell->in_attack_range = selection;
  if (range == 0) {
    return;
  }
  int x = start_cell->x;
  int y = start_cell->y;
  int x_limit = grid->max_grid_cells_x - 1;
  int y_limit = grid->max_grid_cells_y - 1;
  if (y != 0) {
    Point *cell_up = cell_arr + x + grid->max_grid_cells_x * (y - 1);
    attack_range_calc(grid, cell_arr, cell_up, range - 1, selection);
  }
  if (y < y_limit) {

    Point *cell_down = cell_arr + x + grid->max_grid_cells_x * (y + 1);
    attack_range_calc(grid, cell_arr, cell_down, range - 1, selection);
  }
  if (x != 0) {
    Point *cell_left = cell_arr + x - 1 + grid->max_grid_cells_x * (y);
    attack_range_calc(grid, cell_arr, cell_left, range - 1, selection);
  }
  if (x < x_limit) {
    Point *cell_right = cell_arr + x + 1 + grid->max_grid_cells_x * (y);
    attack_range_calc(grid, cell_arr, cell_right, range - 1, selection);
  }

  return;
}

void spread_terrain(GridConfig * grid, Point *cell_arr, Point *start_cell, int range,
                    Terrain terrain) {
  // Set current cell's terrain
  start_cell->terrain = terrain;

  // Base case: stop spreading
  if (range == 0) {
    return;
  }

  int x = start_cell->x;
  int y = start_cell->y;
  int x_limit = grid->max_grid_cells_x - 1;
  int y_limit = grid->max_grid_cells_y - 1;

  // Spread to adjacent cells (up, down, left, right)
  if (y != 0) {
    Point *cell_up = cell_arr + x + grid->max_grid_cells_x * (y - 1);
    spread_terrain(grid, cell_arr, cell_up, range - 1, terrain);
  }
  if (y < y_limit) {
    Point *cell_down = cell_arr + x + grid->max_grid_cells_x * (y + 1);
    spread_terrain(grid, cell_arr, cell_down, range - 1, terrain);
  }
  if (x != 0) {
    Point *cell_left = cell_arr + x - 1 + grid->max_grid_cells_x * y;
    spread_terrain(grid, cell_arr, cell_left, range - 1, terrain);
  }
  if (x < x_limit) {
    Point *cell_right = cell_arr + x + 1 + grid->max_grid_cells_x * y;
    spread_terrain(grid, cell_arr, cell_right, range - 1, terrain);
  }
}

Point *get_random_cell(GridConfig * grid, Point *cell_arr) {
  int rand_x = rand() % grid->max_grid_cells_x;
  int rand_y = rand() % grid->max_grid_cells_y;
  return cell_arr + rand_x + rand_y * grid->max_grid_cells_x;
}

void generate_biome_cores(GridConfig * grid, Point *cell_arr, BiomeConfig config) {
  // since modulo is offset
  int num_cores = rand() % (config.max_cores + 1);

  for (int i = 0; i < num_cores; i++) {
    Point *core = get_random_cell(grid, cell_arr);
    int range = (rand() % config.max_range) + 1;
    spread_terrain(grid, cell_arr, core, range, config.terrain);
  }
}

void generate_all_biomes(GridConfig * grid, Point *cell_arr, BiomeConfig *biome_configs,
                         int num_biomes, int layers) {
  for (int layer = 0; layer < layers; layer++) {
    for (int i = 0; i < num_biomes; i++) {
      generate_biome_cores(grid, cell_arr, biome_configs[i]);
    }
  }
}

void actor_init(Actor *actor, Faction *owner, Texture2D sprite) {
  actor->sprite = sprite;
  strcpy(actor->name, "Azao");
  actor->owner = owner;
  actor->can_move = true;
  actor->can_act = true;

  actor->level = 1;
  actor->next_level_xp = 100;

  actor->max_health = 20;
  actor->curr_health = 20;

  actor->movement = 4;

  actor->attack = 8;
  actor->defense = 3;

  actor->magic_defense = 3;
  actor->magic_attack = 2;

  actor->range = 1;
}

void cell_flag_flush(Point *cell_arr, GridConfig * grid) {
  for (int yCoor = 0; yCoor < grid->max_grid_cells_y; yCoor++) {
    for (int xCoor = 0; xCoor < grid->max_grid_cells_x; xCoor++) {
      cell_arr[xCoor + yCoor * grid->max_grid_cells_x].in_range = false;
      cell_arr[xCoor + yCoor * grid->max_grid_cells_x].in_attack_range = false;
    }
  }
}

// redefining cell point caused the game not to start sometimes
Point *get_random_spawn_cell(GridConfig * grid, Point *cell_arr) {
  Point *cell = get_random_cell(grid, cell_arr);
  while (cell->terrain.id == 2 || cell->occupant != NULL) {
    cell = get_random_cell(grid, cell_arr);
    // above Point * cell caused game to not start sometimes
  }
  return cell;
}

GridConfig * grid_init( int g_off_x, int g_off_y, int g_cell_size,
                  int max_cell_x, int max_cell_y) {

    GridConfig * grid = malloc(sizeof(GridConfig));
    grid->grid_offset_x = g_off_x;
    grid->grid_offset_y = g_off_y;

    grid->grid_cell_size = g_cell_size;

    grid->max_grid_cells_x = max_cell_x;
    grid->max_grid_cells_y = max_cell_y;
    return grid;
}