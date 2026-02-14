#ifndef GAME_CLEANUP_H_
#define GAME_CLEANUP_H_

#include "game/actions.h"
#include "game/game_logic.h"
#include "render/structure_sprites.h"
#include "render/unit_sprites.h"
#include "types.h"

/**
 * @brief Bundle of every resource allocated when entering PLAYING mode.
 *
 * A file-static instance lives in main.c and is registered with atexit()
 * so that resources are freed on any exit path (including fatal errors).
 */
typedef struct AppResources {
  Point *mapArr;
  Faction *factions;
  int num_factions;
  GameState *game_state;
  UnitSprites *unit_sprites;
  StructureSprites *structure_sprites;
  ActionIcons action_icons;
  Terrain *terrains;
  int terrain_count;
  GridConfig *grid_config;
  int initialized; /**< Guard: 1 when resources are live */
} AppResources;

/**
 * @brief atexit-compatible cleanup handler.
 *
 * Checks the file-static AppResources; if resources are initialised,
 * frees them all and marks them as released. Safe to call repeatedly.
 */
void app_resources_cleanup_atexit(void);

/**
 * @brief Store a pointer to the AppResources instance that should be
 *        freed on abnormal exit.  Must be called once from main().
 *
 * @param res Pointer to the AppResources struct (must remain valid for
 *            the lifetime of the program).
 */
void app_resources_register(AppResources *res);

#endif
