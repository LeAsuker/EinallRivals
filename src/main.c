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
#include "combat.h"
#include "game_logic.h"
#include <assert.h>

int main(void) {
  Image sea_sprite = LoadImage("resources/sea_ter.png");
  Image mountains_sprite = LoadImage("resources/mountain_ter2.png");
  Image plains_sprite = LoadImage("resources/plains_ter.png");
  Image arctic_sprite = LoadImage("resources/arctic_ter.png");
  Image forest_sprite = LoadImage("resources/forest_ter2.png");

  Image deep_forest_sprite = LoadImage("resources/deep_forest_ter.png");
  Image deep_sea_sprite = LoadImage("resources/deep_sea_ter.png");

  Image player_base_sprite = LoadImage("resources/base_ter.png");

  Image v_militia_sprite = LoadImage("resources/ventus_militia.png");
  Image d_militia_sprite = LoadImage("resources/darkus_militia.png");

  ImageResize(&sea_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&mountains_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&plains_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&arctic_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&forest_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);

  ImageResize(&deep_forest_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&deep_sea_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);

  ImageResize(&player_base_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);

  
  ImageResize(&d_militia_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  ImageResize(&v_militia_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
  
  // Define terrain types
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 1600;
  const int screenHeight = 900;
  srand(time(NULL));
  
  GridConfig * grid_config = grid_init( GRID_OFFSET_X, GRID_OFFSET_Y, GRID_CELL_SIZE,
    MAX_GRID_CELLS_X, MAX_GRID_CELLS_Y);
    
    // initwindow creates opengl context, texture stuff needs to happen after it
  InitWindow(screenWidth, screenHeight, "WaterEmblemProto");

  Terrain None = {
    .id = -1, .color = WHITE, .passable = false, .deep_version = NULL
  };
  Terrain DeepForest = {
    .id = 41, .color = BLACK, .passable = false, .deep_version = &None,
    .sprite = LoadTextureFromImage(deep_forest_sprite)
  };

  Terrain DeepSea = {
    .id = 21, .color = DARKBLUE, .passable = false, .deep_version = &None,
    .sprite = LoadTextureFromImage(deep_sea_sprite)
  };
  Terrain Plains = {
    .id = 0,
    .color = GREEN,
    .sprite = LoadTextureFromImage(plains_sprite),
    .passable = true,
    .deep_version = &None };

  Terrain Mountains = {.id = 1,
                       .color = LIGHTGRAY,
                       .sprite = LoadTextureFromImage(mountains_sprite),
                       .passable = false,
                       .deep_version = &None };

  Terrain Sea = {
      .id = 2, .color = BLUE, .sprite = LoadTextureFromImage(sea_sprite),
      .passable = false, .deep_version = &DeepSea};

  Terrain Arctic = {
      .id = 3, .color = WHITE, .sprite = LoadTextureFromImage(arctic_sprite),
      .passable = true, .deep_version = &Mountains };

  Terrain Forest = {.id = 4,
                    .color = DARKGREEN,
                    .sprite = LoadTextureFromImage(forest_sprite),
                    .passable = true,
                    .deep_version = &DeepForest};

  Terrain Coast = {
    .id = 5, .color = YELLOW, .passable = true, .deep_version = &None
  };

  Terrain PlayerBase = {
    .id = 6, .color = ORANGE, .passable = true, .deep_version = &None,
    .sprite = LoadTextureFromImage(player_base_sprite)
  };


  strcpy(Plains.name, "Plains");
  strcpy(Mountains.name, "Mountains");
  strcpy(Sea.name, "Sea");
  strcpy(Arctic.name, "Hills");
  strcpy(Forest.name, "Forest");
  strcpy(DeepForest.name, "DeepForest");
  strcpy(DeepSea.name, "DeepSea");

  // Configure each biome type
  BiomeConfig biome_configs[] = {
      {.terrain = Arctic, .max_cores = 3, .max_range = 4},
      {.terrain = Forest, .max_cores = 5, .max_range = 4},
      {.terrain = Sea, .max_cores = 2, .max_range = 5}};

  // map init

  Point *mapArr = map_create(grid_config, Plains);

  int num_biomes = sizeof(biome_configs) / sizeof(BiomeConfig);
  int layers = 7;

  map_generate_all_biomes(grid_config, mapArr, biome_configs, num_biomes, layers);

  map_generate_deep_ter(mapArr, grid_config);

  // base building
  map_spread_terrain(grid_config, mapArr, map_get_cell(mapArr, grid_config, 0, 0), 3, PlayerBase);
  map_spread_terrain(grid_config, mapArr, map_get_cell(mapArr, grid_config, grid_config->max_grid_cells_x - 1,
      grid_config->max_grid_cells_y - 1), 3, PlayerBase);

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

  int num_factions = sizeof(factions) / sizeof(Faction);

  Texture2D d_militia_text = LoadTextureFromImage(d_militia_sprite);
  Texture2D v_militia_text = LoadTextureFromImage(v_militia_sprite);

  Actor *dark_troops = actor_array_create(DARK_TROOP_NUM, &factions[0], d_militia_text);
  Actor *vent_troops = actor_array_create(VENT_TROOP_NUM, &factions[1], v_militia_text);

  for (int i = 0; i < DARK_TROOP_NUM; i++) {
    actor_init(dark_troops + i, factions + 0, d_militia_text);
    Point *spawn = map_get_random_corner_spawn_cell(mapArr, grid_config, 0, 4, 16);
    spawn->occupant = dark_troops + i;
  }
  
  for (int i = 0; i < VENT_TROOP_NUM; i++) {
    actor_init(vent_troops + i, factions + 1, v_militia_text);
    Point *spawn = map_get_random_corner_spawn_cell(mapArr, grid_config, 2, 4, 16);
    spawn->occupant = vent_troops + i;
  }
  // Init current player state


  GameState *game_state = game_state_create(factions, num_factions);

  TroopGroup troop_groups[] = {
      troop_group_create(dark_troops, DARK_TROOP_NUM, &factions[0]),
      troop_group_create(vent_troops, VENT_TROOP_NUM, &factions[1])
  };

  int num_groups = sizeof(troop_groups) / sizeof(TroopGroup);

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
    if (game_is_over(game_state)) {
      // Game over - just render and wait for window close
      render_game(&render_ctx, mapArr, input_state.focused_cell, 
                  game_state->winner ? game_state->winner->name : "Game Over");
      continue;
    } 

    input_update(&input_state, grid_config, mapArr);
    if (input_state.left_click) {
      input_handle_selection(&input_state, grid_config, mapArr);
    }

    if (input_state.right_click) {
      input_handle_movement(&input_state, grid_config, mapArr);
    }

    if (input_state.end_turn_requested) {
      game_end_current_turn(game_state, troop_groups, num_groups);
    }

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    Faction *current_faction = game_get_current_faction(game_state);
    render_game(&render_ctx, mapArr, input_state.focused_cell, current_faction->name);
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  map_free(mapArr);
  actor_array_free(dark_troops, DARK_TROOP_NUM);
  actor_array_free(vent_troops, VENT_TROOP_NUM);
  game_state_free(game_state);
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
