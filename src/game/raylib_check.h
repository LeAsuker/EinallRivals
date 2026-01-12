/*
 * raylib_check.h
 *
 * Convenience macros to check return values from raylib calls and handle
 * fatal failures in a consistent way (log, cleanup and exit).
 */
#ifndef RAYLIB_CHECK_H_
#define RAYLIB_CHECK_H_

#include "../types.h"
#include "raylib.h"
#include <stdio.h>

/* Forward declaration: the project must provide an implementation that
 * performs any necessary cleanup and exits. A minimal implementation that
 * closes the window and exits is provided in `src/game/raylib_check.c`.
 */
void game_fatal_cleanup_and_exit(const char *context);

/* Check that a pointer-returning raylib call didn't return NULL. */
#define RAYLIB_CHECK_NOT_NULL(expr)                                            \
  do {                                                                        \
    void *_r = (void *)(expr);                                                \
    if (_r == NULL) {                                                         \
      if (DEBUG_LOG)                                                          \
        fprintf(stderr, "[FATAL] Raylib call '%s' returned NULL in %s\n",   \
                #expr, __func__);                                             \
      game_fatal_cleanup_and_exit(#expr);                                     \
    }                                                                         \
  } while (0)

/* Check that an integer/bool-style raylib call returned true/non-zero. */
#define RAYLIB_CHECK_TRUE(expr)                                                \
  do {                                                                        \
    int _r = (expr);                                                          \
    if (!_r) {                                                                \
      if (DEBUG_LOG)                                                          \
        fprintf(stderr, "[FATAL] Raylib call '%s' returned false/0 in %s\n",\
                #expr, __func__);                                             \
      game_fatal_cleanup_and_exit(#expr);                                     \
    }                                                                         \
  } while (0)

/* Check that a Texture2D returned by a raylib call is valid (id != 0). */
#define RAYLIB_CHECK_TEXTURE(expr)                                             \
  do {                                                                        \
    Texture2D _t = (expr);                                                    \
    if (_t.id == 0) {                                                         \
      if (DEBUG_LOG)                                                          \
        fprintf(stderr, "[FATAL] Raylib call '%s' produced invalid Texture2D\n", \
                #expr);                                                        \
      game_fatal_cleanup_and_exit(#expr);                                     \
    }                                                                         \
  } while (0)

/* Check that an Image- or Wave-like struct that exposes a 'data' pointer
 * successfully contains data. (Uses typeof so a variety of raylib types
 * can be used.) */
#define RAYLIB_CHECK_HAS_DATA(expr)                                            \
  do {                                                                        \
    __typeof__(expr) _v = (expr);                                              \
    if (_v.data == NULL) {                                                    \
      if (DEBUG_LOG)                                                          \
        fprintf(stderr, "[FATAL] Raylib call '%s' returned empty data\n",    \
                #expr);                                                        \
      game_fatal_cleanup_and_exit(#expr);                                     \
    }                                                                         \
  } while (0)

#endif
