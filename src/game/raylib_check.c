#include "raylib_check.h"
#include "game/cleanup.h"
#include <stdlib.h>

/* Minimal fatal cleanup used when raylib reports an unrecoverable error.
 * This attempts to close the raylib window and terminates the process.
 * If you prefer a fuller cleanup (freeing game resources), replace this
 * implementation to call your resource-freeing helpers or make them
 * accessible here. */
void game_fatal_cleanup_and_exit(const char *context) {
  if (DEBUG_LOG)
    fprintf(stderr, "[FATAL] Raylib failure: %s\n", context);

  /* Close the window if it's open and exit with failure. */
  /* Ask the program to perform a global cleanup (if available) before
   * forcing the window closed and exiting. This gives the app a chance
   * to free game resources cleanly on fatal raylib errors. */
  game_global_cleanup_on_fatal();
  CloseWindow();
  exit(EXIT_FAILURE);
}
