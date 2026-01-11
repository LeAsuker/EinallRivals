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
#include "game/spawning.h"
#include "game/actions.h"
#include "ui/modal.h"

typedef enum {
  GAME_MODE_MENU,
  GAME_MODE_PLAYING,
  GAME_MODE_QUIT
} GameMode;

// Forward declaration for cleanup function
static void cleanup_game_resources(Point *mapArr, Faction *factions, int num_factions,
                                   GameState *game_state, UnitSprites *unit_sprites,
                                   StructureSprites *structure_sprites, 
                                   Terrain *terrains, int terrain_count,
                                   GridConfig *grid_config);

int main(void) {
  const int screenWidth = 1600;
  const int screenHeight = 1000;
  srand(time(NULL));

  InitWindow(screenWidth, screenHeight, "WaterEmblemProto");
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);  // Disable default ESC to close, we'll handle it ourselves

  GameMode mode = GAME_MODE_MENU;
  
  // Main application loop - handles menu and game
  while (!WindowShouldClose() && mode != GAME_MODE_QUIT) {
    
    if (mode == GAME_MODE_MENU) {
      // Show title screen
      MenuState menu_state;
      menu_init(&menu_state);
      
      while (!WindowShouldClose() && menu_state.is_active) {
        menu_update(&menu_state);
        menu_render(&menu_state, screenWidth, screenHeight);
        
        if (menu_get_selected(&menu_state) == MENU_QUIT) {
          mode = GAME_MODE_QUIT;
          break;
        }
      }
      
      // If player selected "Start Game", transition to playing mode
      if (menu_get_selected(&menu_state) == MENU_START) {
        mode = GAME_MODE_PLAYING;
      }
      
      if (mode != GAME_MODE_PLAYING) {
        continue; // Stay in menu or quit
      }
    }
    
    // Initialize game resources (only when entering PLAYING mode)
    if (mode == GAME_MODE_PLAYING) {
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
      // Load action icons (skills)
          actions_load_icons();

      // Create units
      UnitClass *militia_class = class_get_militia();
      Character *dark_troops = character_array_create_from_class(DARK_TROOP_NUM, &factions[DARKUS], 
                                             unit_sprites.darkus_militia, militia_class);
      Character *vent_troops = character_array_create_from_class(VENT_TROOP_NUM, &factions[VENTUS], 
                                             unit_sprites.ventus_militia, militia_class);
      // Assign character arrays to their owning factions before placing them
      factions[DARKUS].characters = dark_troops;
      factions[DARKUS].character_count = DARK_TROOP_NUM;

      factions[VENTUS].characters = vent_troops;
      factions[VENTUS].character_count = VENT_TROOP_NUM;
      // Place faction troops into their corners
      spawning_place_faction_in_corner(mapArr, grid_config, &factions[DARKUS], 0, 4, 16);
          spawning_place_faction_in_corner(mapArr, grid_config, &factions[VENTUS], 2, 4, 16);

      // Place Warg Lairs first, then spawn Gaia wargs around those lairs
      int lairs = structure_generation_place_warg_lairs(mapArr, grid_config, terrains, TERRAIN_COUNT, structure_sprites);
      int gaia_warg_count = 0;
      Character *gaia_wargs = NULL;
      if (lairs > 0) {
        gaia_wargs = structure_generation_spawn_wargs_around_lairs(mapArr, grid_config, unit_sprites, &factions[GAIA], &gaia_warg_count);
        if (gaia_wargs != NULL && gaia_warg_count > 0) {
          factions[GAIA].characters = gaia_wargs;
          factions[GAIA].character_count = gaia_warg_count;
        }
      }

      int huts = structure_generation_place_abandoned_huts(mapArr, grid_config, terrains, TERRAIN_COUNT, structure_sprites);

      // Initialize game state
      GameState *game_state = game_state_create(factions, num_factions);
      // Initialize modal system
      game_state->modal = modal_create();

      // Initialize input
      InputState input_state;
      input_init(&input_state);
      // Layout persistent UI buttons now that grid_config is available
      input_layout_buttons(&input_state, grid_config);
      bool button_is_pressed = false;

      // Main game loop
      while (!WindowShouldClose() && mode == GAME_MODE_PLAYING) {
        Faction *current_faction = game_get_current_faction(game_state);
        
        if (game_is_over(game_state)) {
          render_game(&render_ctx, mapArr, input_state.focused_cell, 
                           current_faction, &input_state, &input_state.end_turn_button, input_state.action_buttons, ACTION_BUTTON_COUNT, game_state->modal);
          continue;
        }

        // Handle modal input if modal is active
        if (game_state->modal && game_state->modal->active) {
          ModalResult result = modal_update(game_state->modal);
          
          switch (result) {
            case MODAL_RESULT_EXIT_TO_MENU:
              // Cleanup and return to menu - must break immediately to avoid accessing freed memory
              cleanup_game_resources(mapArr, factions, num_factions, game_state,
                                    &unit_sprites, &structure_sprites, terrains,
                                    TERRAIN_COUNT, grid_config);
              mode = GAME_MODE_MENU;
              break;  // This exits the switch, but we need to exit the while loop too
            case MODAL_RESULT_EXIT_TO_DESKTOP:
              mode = GAME_MODE_QUIT;
              break;
            case MODAL_RESULT_CANCEL:
              // Resume game - modal already deactivated
              break;
            case MODAL_RESULT_NONE:
              // Still active, skip game input this frame
              break;
            default:
              if (result >= MODAL_RESULT_CLASS_CHOICE_0 && 
                  result <= MODAL_RESULT_CLASS_CHOICE_MAX) {
                // Handle class promotion
                int choice = result - MODAL_RESULT_CLASS_CHOICE_0;
                // TODO: Apply promotion logic
                printf("Class choice %d selected\n", choice);
              }
              break;
          }
          
          // Check if we're exiting game mode - if so, break out immediately without rendering
          if (mode != GAME_MODE_PLAYING) {
            break;  // Exit the game loop immediately after cleanup
          }
          
          // Skip normal game input when modal is active and render
          render_game(&render_ctx, mapArr, input_state.focused_cell,
               current_faction, &input_state, &input_state.end_turn_button, input_state.action_buttons, ACTION_BUTTON_COUNT, game_state->modal);
          continue;
        }

        // Check for ESC key to open pause menu (only if modal is not already active)
        if (IsKeyPressed(KEY_ESCAPE) && (!game_state->modal || !game_state->modal->active)) {
          modal_setup_esc_menu(game_state->modal, screenWidth, screenHeight);
          printf("ESC pressed - modal activated: %d\n", game_state->modal->active);
          // Render immediately with modal visible
          render_game(&render_ctx, mapArr, input_state.focused_cell,
               current_faction, &input_state, &input_state.end_turn_button, input_state.action_buttons, ACTION_BUTTON_COUNT, game_state->modal);
          continue;
        }

        input_update(&input_state, grid_config, mapArr);
        
        // If it's an AI faction's turn, process AI actions automatically
        if (!game_is_player_turn(game_state) && !game_is_over(game_state)) {
          game_process_ai_turn(game_state, mapArr, grid_config);
          continue; // skip player input/render frame; AI processing and turn advancement handled
        }
        // Let input subsystem process action clicks (executes skills when applicable)
        input_handle_action_click(&input_state, grid_config, mapArr);
        // Use the End Turn button's pressed state (updated in input_update)
        button_is_pressed = input_state.end_turn_button.pressed;
        
        if (input_state.left_click) {
          // Handle left-click-based control (movement, actions, focus)
          input_handle_left_click(&input_state, grid_config, mapArr);
        }

        if (input_state.end_turn_requested) {
          game_end_current_turn(game_state);
        }

        // renders only after first click to avoid null focused_cell
        render_game(&render_ctx, mapArr, input_state.focused_cell,
             current_faction, &input_state, &input_state.end_turn_button, input_state.action_buttons, ACTION_BUTTON_COUNT, game_state->modal);
      }
      
      // Cleanup game resources after exiting game loop
      // Only cleanup if we didn't already return to menu (which already cleaned up)
      if (mode == GAME_MODE_PLAYING) {
        // We're quitting directly from game without going through menu
        cleanup_game_resources(mapArr, factions, num_factions, game_state,
                              &unit_sprites, &structure_sprites, terrains,
                              TERRAIN_COUNT, grid_config);
      }
      // If mode == GAME_MODE_MENU, cleanup already happened in modal handler
    }
  }

  // Final cleanup and window close
  CloseWindow();
  return 0;
}

// Cleanup function for game resources
static void cleanup_game_resources(Point *mapArr, Faction *factions, int num_factions,
                                   GameState *game_state, UnitSprites *unit_sprites,
                                   StructureSprites *structure_sprites, 
                                   Terrain *terrains, int terrain_count,
                                   GridConfig *grid_config) {
  map_free(mapArr);
  factions_free_actors(factions, num_factions);
  actions_unload_icons();
  if (game_state->modal) {
    modal_free(game_state->modal);
  }
  game_state_free(game_state);
  unit_sprites_unload(unit_sprites);
  structure_sprites_unload(structure_sprites);
  terrain_unload_all(terrains, terrain_count);
  free(grid_config);
}
