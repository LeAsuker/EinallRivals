#include "raylib.h"

#include <stdio.h>
#include <stdlib.h> // Required for: calloc(), free()
#include <string.h> // Required for: memcpy(), strcmp()
#include <time.h>

#include "main.h"
#include "types.h"
#include "rendering.h"
#include "utils.h"
#include "input.h"
#include "map.h"
#include "actor.h"

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

  Point *mapArr = map_create(grid_config, Plains);

  int num_biomes = sizeof(biome_configs) / sizeof(BiomeConfig);
  int layers = 7;
  map_generate_all_biomes(grid_config, mapArr, biome_configs, num_biomes, layers);

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
    Point *spawn = map_get_random_spawn_cell(mapArr, grid_config);
    spawn->occupant = dark_troops + i;
  }
  
  for (int i = 0; i < VENT_TROOP_NUM; i++) {
    actor_init(vent_troops + i, factions + 1, v_militia_text);
    Point *spawn = map_get_random_spawn_cell(mapArr, grid_config);
    spawn->occupant = vent_troops + i;
  }
  // Init current player state

  Faction *curr_faction = factions + 0;
  Point *focused_cell = NULL;

  InputState input_state; // object creation
  input_init(&input_state); // initialization, is made to work, is given attributes

  SetTargetFPS(60);
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------
    // The XY coords are in the top left corner of the square
    // LMB
    input_update(&input_state, grid_config, mapArr);
    if (input_state.left_click) {
      input_handle_selection(&input_state, grid_config, mapArr);
    }

    if (input_state.right_click) {
      input_handle_movement(&input_state, grid_config, mapArr);
    }

    if (input_state.end_turn_requested) {
      // TODO: Implement turn system here
      // For now, just a placeholder
      printf("End turn requested!\n");
    }

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    render_game(&render_ctx, mapArr, input_state.focused_cell, curr_faction->name);
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  map_free(mapArr);
  free(dark_troops);
  free(vent_troops);
  free(grid_config);

  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

// still used in input.c

void cell_flag_flush(Point *cell_arr, GridConfig * grid) {
  for (int yCoor = 0; yCoor < grid->max_grid_cells_y; yCoor++) {
    for (int xCoor = 0; xCoor < grid->max_grid_cells_x; xCoor++) {
      cell_arr[xCoor + yCoor * grid->max_grid_cells_x].in_range = false;
      cell_arr[xCoor + yCoor * grid->max_grid_cells_x].in_attack_range = false;
    }
  }
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