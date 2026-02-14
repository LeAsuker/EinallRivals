#include "raylib_check.h"
#include "game/cleanup.h"
#include <stdlib.h>

/* Minimal fatal cleanup used when raylib reports an unrecoverable error.
 * This attempts to close the raylib window and terminates the process.
 * The atexit handler registered in main() will free game resources
 * automatically. */
void game_fatal_cleanup_and_exit(const char *context) {
  if (DEBUG_LOG)
    fprintf(stderr, "[FATAL] Raylib failure: %s\n", context);

  /* Ask the program to perform cleanup before forcing the window closed
   * and exiting. The atexit handler will free game resources if they are
   * currently initialised. */
  app_resources_cleanup_atexit();
  CloseWindow();
  exit(EXIT_FAILURE);
}
