#include "raylib.h"
#include "resource_dir.h"

#include <stdio.h>
#include <stdlib.h> // Required for: calloc(), free()
#include <string.h> // Required for: memcpy(), strcmp()
#include <time.h>

#include "core/utils.h"
#include "game/actions.h"
#include "game/actor.h"
#include "game/biome_config.h"
#include "game/combat.h"
#include "game/faction_init.h"
#include "game/game_logic.h"
#include "game/map.h"
#include "game/spawning.h"
#include "game/structure.h"
#include "game/structure_generation.h"
#include "game/terrain.h"
#include "input/input.h"
#include "main.h"
#include "render/rendering.h"
#include "render/structure_sprites.h"
#include "render/unit_sprites.h"
#include "types.h"
#include "ui/menu.h"
#include "ui/modal.h"

#include "game/cleanup.h"

// File-static AppResources pointer registered with atexit for cleanup.
static AppResources *s_app_resources = NULL;

/**
 * @brief Store a pointer to the AppResources that the atexit handler
 *        should clean up.
 */
void app_resources_register(AppResources *res) { s_app_resources = res; }

typedef enum { GAME_MODE_MENU, GAME_MODE_PLAYING, GAME_MODE_QUIT } GameMode;

// Forward declarations
static void cleanup_game_resources(AppResources *res);
static GameMode game_loop_playing(GridConfig *grid_config, Point *mapArr,
                                  GameState *game_state,
                                  RenderContext *render_ctx,
                                  InputState *input_state,
                                  AppResources *app_res, int screenWidth,
                                  int screenHeight);

/**
 * @brief Program entry point: initialize window, run main menu and game loops.
 * @return Exit code (0 on normal termination).
 */
int main(void) {
  const int screenWidth = 1600;
  const int screenHeight = 1000;
  srand(time(NULL));

  InitWindow(screenWidth, screenHeight, "WaterEmblemProto");
  SetTargetFPS(60);
  SetExitKey(
      KEY_NULL); // Disable default ESC to close, we'll handle it ourselves

  // Find and set the resources directory
  if (!SearchAndSetResourceDir("resources")) {
    TraceLog(LOG_WARNING, "Could not find resources directory, using current working directory");
  }

  GameMode mode = GAME_MODE_MENU;

  // Set up the resource bundle and register with atexit for safe cleanup.
  AppResources app_res = {0};
  app_resources_register(&app_res);
  atexit(app_resources_cleanup_atexit);

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
      GridConfig *grid_config =
          grid_init(GRID_OFFSET_X, GRID_OFFSET_Y, GRID_CELL_SIZE,
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
      map_generate_all_biomes(grid_config, mapArr, biome_configs, num_biomes,
                              layers);
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
      StructureSprites structure_sprites =
          structure_sprites_load(GRID_CELL_SIZE);
      // Load action icons (skills)
      app_res.action_icons = action_icons_load();

      // Create units
      Character *dark_troops = character_array_create_from_class(
          DARK_TROOP_NUM, &factions[DARKUS], unit_sprites.darkus_militia,
          ARCHETYPE_WARRIOR, &app_res.action_icons);
      Character *vent_troops = character_array_create_from_class(
          VENT_TROOP_NUM, &factions[VENTUS], unit_sprites.ventus_militia,
          ARCHETYPE_WARRIOR, &app_res.action_icons);
      // Assign character arrays to their owning factions before placing them
      factions[DARKUS].characters = dark_troops;
      factions[DARKUS].character_count = DARK_TROOP_NUM;

      factions[VENTUS].characters = vent_troops;
      factions[VENTUS].character_count = VENT_TROOP_NUM;
      // Place faction troops into their corners
      spawning_place_faction_in_corner(mapArr, grid_config, &factions[DARKUS],
                                       0, 4, 16);
      spawning_place_faction_in_corner(mapArr, grid_config, &factions[VENTUS],
                                       2, 4, 16);

      // Place Warg Lairs first, then spawn Gaia wargs around those lairs
      int lairs = structure_generation_place_warg_lairs(
          mapArr, grid_config, terrains, TERRAIN_COUNT, structure_sprites);
      int gaia_warg_count = 0;
      Character *gaia_wargs = NULL;
      if (lairs > 0) {
        gaia_wargs = structure_generation_spawn_wargs_around_lairs(
            mapArr, grid_config, unit_sprites, &factions[GAIA],
            &gaia_warg_count, &app_res.action_icons);
        if (gaia_wargs != NULL && gaia_warg_count > 0) {
          factions[GAIA].characters = gaia_wargs;
          factions[GAIA].character_count = gaia_warg_count;
        }
      }

      int huts = structure_generation_place_abandoned_huts(
          mapArr, grid_config, terrains, TERRAIN_COUNT, structure_sprites);

      // Initialize game state
      GameState *game_state = game_state_create(factions, num_factions);
      // Initialize modal system
      game_state->modal = modal_create();

      // Initialize input
      InputState input_state;
      input_init(&input_state);
      // Layout persistent UI buttons now that grid_config is available
      input_layout_buttons(&input_state, grid_config);
      /* Register current resources for potential atexit/fatal cleanup. */
      app_res.mapArr = mapArr;
      app_res.factions = factions;
      app_res.num_factions = num_factions;
      app_res.game_state = game_state;
      app_res.unit_sprites = &unit_sprites;
      app_res.structure_sprites = &structure_sprites;
      app_res.terrains = terrains;
      app_res.terrain_count = TERRAIN_COUNT;
      app_res.grid_config = grid_config;
      app_res.initialized = 1;

      // Main game loop
      while (!WindowShouldClose() && mode == GAME_MODE_PLAYING) {
        mode = game_loop_playing(grid_config, mapArr, game_state, &render_ctx,
                                 &input_state, &app_res, screenWidth,
                                 screenHeight);
      }

      // Cleanup game resources after exiting game loop
      // Only cleanup if we didn't already return to menu (which already cleaned
      // up)
      if (mode == GAME_MODE_PLAYING) {
        // We're quitting directly from game without going through menu
        cleanup_game_resources(&app_res);
      }
      // If mode == GAME_MODE_MENU, cleanup already happened in modal handler
    }
  }

  // Final cleanup and window close
  CloseWindow();
  return 0;
}

/**
 * @brief Run one frame of the PLAYING game loop.
 *
 * Handles modal input, ESC menu, player input, AI turns, and rendering.
 * Returns the current or updated game mode (may switch to MENU or QUIT).
 */
static GameMode game_loop_playing(GridConfig *grid_config, Point *mapArr,
                                  GameState *game_state,
                                  RenderContext *render_ctx,
                                  InputState *input_state,
                                  AppResources *app_res, int screenWidth,
                                  int screenHeight) {
  GameMode mode = GAME_MODE_PLAYING;
  Faction *current_faction = game_get_current_faction(game_state);

  if (game_is_over(game_state)) {
    render_game(render_ctx, mapArr, input_state->focused_cell, current_faction,
                input_state, &input_state->end_turn_button,
                input_state->action_buttons, ACTION_BUTTON_COUNT,
                game_state->modal);
    return mode;
  }

  // Handle modal input if modal is active
  if (game_state->modal && game_state->modal->active) {
    ModalResult result = modal_update(game_state->modal);

    switch (result) {
    case MODAL_RESULT_EXIT_TO_MENU:
      cleanup_game_resources(app_res);
      mode = GAME_MODE_MENU;
      break;
    case MODAL_RESULT_EXIT_TO_DESKTOP:
      mode = GAME_MODE_QUIT;
      break;
    case MODAL_RESULT_CANCEL:
      break;
    case MODAL_RESULT_NONE:
      break;
    default:
      if (result >= MODAL_RESULT_CLASS_CHOICE_0 &&
          result <= MODAL_RESULT_CLASS_CHOICE_MAX) {
        int choice = result - MODAL_RESULT_CLASS_CHOICE_0;
        Character *character = (Character *)game_state->modal->data;
        if (character != NULL && character->unit_class != NULL) {
          const UnitClass *new_class =
              character->unit_class->class_tree.promotions[choice];
          if (new_class != NULL) {
            Texture2D new_sprite = {0};
            if (strcmp(new_class->name, "Spearman") == 0) {
              if (strcmp(character->owner->name, "Darkus") == 0)
                new_sprite = app_res->unit_sprites->darkus_spearman;
              else if (strcmp(character->owner->name, "Ventus") == 0)
                new_sprite = app_res->unit_sprites->ventus_spearman;
            } else if (strcmp(new_class->name, "Swordsman") == 0) {
              if (strcmp(character->owner->name, "Darkus") == 0)
                new_sprite = app_res->unit_sprites->darkus_swordsman;
              else if (strcmp(character->owner->name, "Ventus") == 0)
                new_sprite = app_res->unit_sprites->ventus_swordsman;
            }
            character_promote(character, new_class, new_sprite,
                              &app_res->action_icons);
            character_level_up(character);
          }
        }
      }
      break;
    }

    if (mode != GAME_MODE_PLAYING) {
      return mode;
    }

    render_game(render_ctx, mapArr, input_state->focused_cell, current_faction,
                input_state, &input_state->end_turn_button,
                input_state->action_buttons, ACTION_BUTTON_COUNT,
                game_state->modal);
    return mode;
  }

  // Check for ESC key to open pause menu
  if (IsKeyPressed(KEY_ESCAPE) &&
      (!game_state->modal || !game_state->modal->active)) {
    modal_setup_esc_menu(game_state->modal, screenWidth, screenHeight);
    printf("ESC pressed - modal activated: %d\n", game_state->modal->active);
    render_game(render_ctx, mapArr, input_state->focused_cell, current_faction,
                input_state, &input_state->end_turn_button,
                input_state->action_buttons, ACTION_BUTTON_COUNT,
                game_state->modal);
    return mode;
  }

  // If a player-controlled unit has a pending level-up with promotion choices,
  // open the promotion modal automatically.
  if (game_is_player_turn(game_state)) {
    Character *pending_promo = NULL;
    Faction *cf = game_get_current_faction(game_state);
    if (cf != NULL) {
      for (int i = 0; i < cf->character_count; i++) {
        Character *c = &cf->characters[i];
        if (c->level_up_pending && c->unit_class != NULL &&
            c->unit_class->class_tree.promotion_count > 0) {
          pending_promo = c;
          break;
        }
      }
    }
    if (pending_promo != NULL && game_state->modal != NULL &&
        !game_state->modal->active) {
      const char *options[4];
      int option_count = pending_promo->unit_class->class_tree.promotion_count;
      for (int i = 0; i < option_count; i++) {
        const UnitClass *opt =
            pending_promo->unit_class->class_tree.promotions[i];
        options[i] = (opt != NULL) ? opt->name : "?";
      }
      modal_setup_level_up(game_state->modal, screenWidth, screenHeight,
                           pending_promo, options, option_count);
    }
  }

  input_update(input_state, grid_config, mapArr);

  // AI turn processing
  if (!game_is_player_turn(game_state) && !game_is_over(game_state)) {
    game_process_ai_turn(game_state, mapArr, grid_config);
    return mode;
  }

  input_handle_action_click(input_state, grid_config, mapArr);

  if (input_state->left_click) {
    input_handle_left_click(input_state, grid_config, mapArr);
  }

  if (input_state->end_turn_requested) {
    game_end_current_turn(game_state);
  }

  render_game(render_ctx, mapArr, input_state->focused_cell, current_faction,
              input_state, &input_state->end_turn_button,
              input_state->action_buttons, ACTION_BUTTON_COUNT,
              game_state->modal);

  return mode;
}

/**
 * @brief Free all game resources tracked by an AppResources bundle.
 *
 * After freeing, the bundle is zeroed so that subsequent calls are safe.
 *
 * @param res AppResources to release (NULL-safe; no-op if not initialised).
 */
static void cleanup_game_resources(AppResources *res) {
  if (res == NULL || !res->initialized) {
    return;
  }
  map_free(res->mapArr);
  factions_free_actors(res->factions, res->num_factions);
  action_icons_unload(&res->action_icons);
  if (res->game_state != NULL && res->game_state->modal != NULL) {
    modal_free(res->game_state->modal);
  }
  game_state_free(res->game_state);
  unit_sprites_unload(res->unit_sprites);
  structure_sprites_unload(res->structure_sprites);
  terrain_unload_all(res->terrains, res->terrain_count);
  free(res->grid_config);
  *res = (AppResources){0};
}

/* atexit-compatible handler that uses the file-static AppResources pointer
 * registered by main() via app_resources_register(). */
void app_resources_cleanup_atexit(void) {
  if (s_app_resources == NULL) {
    return;
  }
  cleanup_game_resources(s_app_resources);
}
