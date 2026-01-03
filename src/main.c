#include "raylib.h"

#include <stdio.h>
#include <stdlib.h> // Required for: calloc(), free()
#include <string.h> // Required for: memcpy(), strcmp()
#include <time.h>

#include "main.h"
#include "types.h"
#include "render/rendering.h"
#include "core/utils.h"
#include "input/input.h"
#include "game/map.h"
#include "game/actor.h"
#include "game/structure.h"
#include "game/combat.h"
#include "game/game_logic.h"
#include "ui/menu.h"
#include "game/terrain.h"
#include "game/biome_config.h"
#include "render/unit_sprites.h"
#include "render/structure_sprites.h"
#include "game/faction_init.h"
#include "game/structure_generation.h"

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

  // Initialize rendering
  RenderContext render_ctx;
  render_init(&render_ctx, grid_config);

  // Initialize factions
  Faction factions[3];
  int num_factions = faction_init_default(factions, 3);

  // Load unit sprites
  UnitSprites unit_sprites = unit_sprites_load(GRID_CELL_SIZE);
  // Load structure sprites
  StructureSprites structure_sprites = structure_sprites_load(GRID_CELL_SIZE);

  // Create units
  ActorTemplate DEFAULT_MILITIA_TEMPLATE;
  actor_get_default_militia_template(&DEFAULT_MILITIA_TEMPLATE);
  Actor *dark_troops = actor_array_create_from_template(DARK_TROOP_NUM, &factions[0], 
                                         unit_sprites.darkus_militia, &DEFAULT_MILITIA_TEMPLATE);
  Actor *vent_troops = actor_array_create_from_template(VENT_TROOP_NUM, &factions[1], 
                                         unit_sprites.ventus_militia, &DEFAULT_MILITIA_TEMPLATE);
  // Spawn units
  for (int i = 0; i < DARK_TROOP_NUM; i++) {
    Point *spawn = map_get_random_corner_spawn_cell(mapArr, grid_config, 0, 4, 16);
    spawn->occupant = dark_troops + i;
  }
  
  for (int i = 0; i < VENT_TROOP_NUM; i++) {
    Point *spawn = map_get_random_corner_spawn_cell(mapArr, grid_config, 2, 4, 16);
    spawn->occupant = vent_troops + i;
  }

  // Place Warg Lairs and spawn Gaia wargs using the structure generation helper
  int gaia_warg_count = 0;
  Actor *gaia_wargs = structure_generation_place_warg_lairs(
      mapArr, grid_config, terrains, TERRAIN_COUNT,
      structure_sprites, unit_sprites, &factions[2], &gaia_warg_count);

  // Assign actor arrays to their owning factions
  factions[0].actors = dark_troops;
  factions[0].actor_count = DARK_TROOP_NUM;

  factions[1].actors = vent_troops;
  factions[1].actor_count = VENT_TROOP_NUM;

  if (gaia_wargs != NULL && gaia_warg_count > 0) {
    factions[2].actors = gaia_wargs;
    factions[2].actor_count = gaia_warg_count;
  }

  // Initialize game state
  GameState *game_state = game_state_create(factions, num_factions);

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
    
    // If it's an AI faction's turn, process AI actions automatically
    if (!game_is_player_turn(game_state) && !game_is_over(game_state)) {
      game_process_ai_turn(game_state, mapArr, grid_config);
      continue; // skip player input/render frame; AI processing and turn advancement handled
    }
    button_is_pressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT) && 
                        input_is_mouse_over_end_turn_button(grid_config);
    
    if (input_state.left_click) {
      input_handle_selection(&input_state, grid_config, mapArr);
    }

    if (input_state.right_click) {
      input_handle_movement(&input_state, grid_config, mapArr);
    }

    if (input_state.end_turn_requested) {
      game_end_current_turn(game_state);
    }

    render_game_full(&render_ctx, mapArr, input_state.focused_cell,
             current_faction, button_is_pressed);
  }

  // Cleanup
  map_free(mapArr);
  // Free faction-owned actor arrays
  for (int i = 0; i < num_factions; i++) {
    if (factions[i].actors != NULL) {
      actor_array_free(factions[i].actors, factions[i].actor_count);
      factions[i].actors = NULL;
      factions[i].actor_count = 0;
    }
  }
  game_state_free(game_state);
  unit_sprites_unload(&unit_sprites);
  structure_sprites_unload(&structure_sprites);
  terrain_unload_all(terrains, TERRAIN_COUNT);
  free(grid_config);

  // troop_groups removed; no extra cleanup required

  CloseWindow();
  return 0;
}

