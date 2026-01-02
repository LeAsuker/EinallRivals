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
  // Load structure sprites
  StructureSprites structure_sprites = structure_sprites_load(GRID_CELL_SIZE);

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

  // Place between 4 and 6 Warg Lairs on random plains tiles; each spawns 2-3 Gaia wargs
  int num_lairs = (rand() % 3) + 4; // 4..6
  int max_possible_wargs = num_lairs * 3;
  Actor *gaia_wargs = NULL;
  int gaia_warg_count = 0;
  if (max_possible_wargs > 0) {
    gaia_wargs = malloc(sizeof(Actor) * max_possible_wargs);
  }

  int lairs_placed = 0;
  int attempts = 0;
  int offsets[8][2] = {{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}};

  while (lairs_placed < num_lairs && attempts < 1000) {
    attempts++;
    Point *candidate = map_get_random_cell(mapArr, grid_config);
    if (candidate == NULL) continue;
    if (candidate->terrain.id != terrains[TERRAIN_PLAINS].id) continue;
    if (candidate->occupant != NULL || candidate->structure != NULL) continue;

    // Place lair
    Structure *lair = structure_create(structure_sprites.warg_lair, "Warg Lair", false, false);
    if (lair == NULL) continue;
    map_place_structure(mapArr, grid_config, candidate->x, candidate->y, lair);

    // Warg template
    ActorTemplate WARG_TEMPLATE;
    strncpy(WARG_TEMPLATE.name, "Warg", sizeof(WARG_TEMPLATE.name));
    WARG_TEMPLATE.max_health = 16;
    WARG_TEMPLATE.movement = 4;
    WARG_TEMPLATE.attack = 7;
    WARG_TEMPLATE.defense = 2;
    WARG_TEMPLATE.magic_attack = 1;
    WARG_TEMPLATE.magic_defense = 1;
    WARG_TEMPLATE.range = 1;

    int to_spawn = (rand() % 2) + 2; // 2..3
    int spawned = 0;
    for (int o = 0; o < 8 && spawned < to_spawn; o++) {
      int nx = candidate->x + offsets[o][0];
      int ny = candidate->y + offsets[o][1];
      if (!map_is_valid_coords(grid_config, nx, ny)) continue;
      Point *dest = map_get_cell(mapArr, grid_config, nx, ny);
      if (dest == NULL) continue;
      if (!map_can_unit_enter_cell(dest, NULL)) continue;

      // initialize warg actor
      actor_init_from_template(&gaia_wargs[gaia_warg_count], &factions[2], unit_sprites.warg, &WARG_TEMPLATE);
      dest->occupant = &gaia_wargs[gaia_warg_count];
      gaia_warg_count++;
      spawned++;
      if (gaia_warg_count >= max_possible_wargs) break;
    }

    lairs_placed++;
  }

  // Initialize game state
  GameState *game_state = game_state_create(factions, num_factions);

  TroopGroup troop_groups[] = {
      troop_group_create(dark_troops, DARK_TROOP_NUM, &factions[0]),
      troop_group_create(vent_troops, VENT_TROOP_NUM, &factions[1])
  };
  int num_groups = sizeof(troop_groups) / sizeof(TroopGroup);

  // If Gaia wargs were created, add them as a troop group so game logic tracks them
  TroopGroup *troop_groups_ext = NULL;
  int num_groups_ext = num_groups;
  if (gaia_wargs != NULL && gaia_warg_count > 0) {
    troop_groups_ext = malloc(sizeof(TroopGroup) * (num_groups + 1));
    memcpy(troop_groups_ext, troop_groups, sizeof(TroopGroup) * num_groups);
    troop_groups_ext[num_groups] = troop_group_create(gaia_wargs, gaia_warg_count, &factions[2]);
    num_groups_ext = num_groups + 1;
  }

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
      game_process_ai_turn(game_state, troop_groups_ext != NULL ? troop_groups_ext : troop_groups, num_groups_ext, mapArr, grid_config);
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
      game_end_current_turn(game_state, troop_groups_ext != NULL ? troop_groups_ext : troop_groups, num_groups_ext);
    }

    render_game_full(&render_ctx, mapArr, input_state.focused_cell,
             current_faction, button_is_pressed);
  }

  // Cleanup
  map_free(mapArr);
  actor_array_free(dark_troops, DARK_TROOP_NUM);
  actor_array_free(vent_troops, VENT_TROOP_NUM);
  if (gaia_wargs != NULL) free(gaia_wargs);
  game_state_free(game_state);
  unit_sprites_unload(&unit_sprites);
  structure_sprites_unload(&structure_sprites);
  terrain_unload_all(terrains, TERRAIN_COUNT);
  free(grid_config);

  if (troop_groups_ext != NULL) free(troop_groups_ext);

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
