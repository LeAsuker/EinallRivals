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
#include "menu.h"
#include "terrain.h"
#include "biome_config.h"
#include "unit_sprites.h"
#include "faction_init.h"

int main(void) {
  const int screenWidth = 1600;
  const int screenHeight = 900;
  srand(time(NULL));

  InitWindow(screenWidth, screenHeight, "WaterEmblemProto");
  SetTargetFPS(60);

  // Show title screen
  MenuState menu_state;
  menu_init(&menu_state);
  
  while (!WindowShouldClose() && menu_state.is_active) {
    menu_update(&menu_state);
    menu_render(&menu_state, screenWidth, screenHeight);
    
    if (menu_get_selected(&menu_state) == MENU_QUIT) {
      CloseWindow();
      return 0;
    }
  }

  // Initialize grid configuration
  GridConfig *grid_config = grid_init(GRID_OFFSET_X, GRID_OFFSET_Y, GRID_CELL_SIZE,
                                      MAX_GRID_CELLS_X, MAX_GRID_CELLS_Y);

  // Load all terrains
  Terrain terrains[TERRAIN_COUNT];
  terrain_init_all(terrains, GRID_CELL_SIZE);

  // Create biome configurations
  BiomeConfig biome_configs[3];
  int num_biomes = biome_config_get_default(biome_configs, 3, terrains);

  // Initialize map
  Point *mapArr = map_create(grid_config, terrains[TERRAIN_PLAINS]);
  int layers = 7;
  map_generate_all_biomes(grid_config, mapArr, biome_configs, num_biomes, layers);
  map_generate_deep_ter(mapArr, grid_config);

  // Build player bases
  // Player base generation in corners removed (no longer required)

  // Initialize rendering
  RenderContext render_ctx;
  render_init(&render_ctx, grid_config);

  // Initialize factions
  Faction factions[3];
  int num_factions = faction_init_default(factions, 3);

  // Load unit sprites
  UnitSprites unit_sprites = unit_sprites_load(GRID_CELL_SIZE);

  // Create units
  Actor *dark_troops = actor_array_create(DARK_TROOP_NUM, &factions[0], 
                                         unit_sprites.darkus_militia);
  Actor *vent_troops = actor_array_create(VENT_TROOP_NUM, &factions[1], 
                                         unit_sprites.ventus_militia);

  // Spawn units
  for (int i = 0; i < DARK_TROOP_NUM; i++) {
    Point *spawn = map_get_random_corner_spawn_cell(mapArr, grid_config, 0, 4, 16);
    spawn->occupant = dark_troops + i;
  }
  
  for (int i = 0; i < VENT_TROOP_NUM; i++) {
    Point *spawn = map_get_random_corner_spawn_cell(mapArr, grid_config, 2, 4, 16);
    spawn->occupant = vent_troops + i;
  }

  // Initialize game state
  GameState *game_state = game_state_create(factions, num_factions);

  TroopGroup troop_groups[] = {
      troop_group_create(dark_troops, DARK_TROOP_NUM, &factions[0]),
      troop_group_create(vent_troops, VENT_TROOP_NUM, &factions[1])
  };
  int num_groups = sizeof(troop_groups) / sizeof(TroopGroup);

  // Initialize input
  InputState input_state;
  input_init(&input_state);
  bool button_is_pressed = false;

  // Main game loop
  while (!WindowShouldClose()) {
    Faction *current_faction = game_get_current_faction(game_state);
    
    if (game_is_over(game_state)) {
      render_game_full(&render_ctx, mapArr, input_state.focused_cell, 
                       current_faction, false);
      continue;
    }

    input_update(&input_state, grid_config, mapArr);
    
    button_is_pressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT) && 
                        input_is_mouse_over_end_turn_button(grid_config);
    
    if (input_state.left_click) {
      input_handle_selection(&input_state, grid_config, mapArr);
    }

    if (input_state.right_click) {
      input_handle_movement(&input_state, grid_config, mapArr);
    }

    if (input_state.end_turn_requested) {
      game_end_current_turn(game_state, troop_groups, num_groups);
    }

    render_game_full(&render_ctx, mapArr, input_state.focused_cell,
                     current_faction, button_is_pressed);
  }

  // Cleanup
  map_free(mapArr);
  actor_array_free(dark_troops, DARK_TROOP_NUM);
  actor_array_free(vent_troops, VENT_TROOP_NUM);
  game_state_free(game_state);
  unit_sprites_unload(&unit_sprites);
  terrain_unload_all(terrains, TERRAIN_COUNT);
  free(grid_config);

  CloseWindow();
  return 0;
}

// Helper function still used in input.c
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
