#include "game/structure.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocate and initialize a Structure instance.
 *
 * @param sprite Texture used to draw the structure.
 * @param name Human-readable name (copied into the struct).
 * @param passable Whether units can enter the tile occupied by this structure.
 * @param lootable Whether the structure can be looted.
 * @return Allocated Structure pointer, or NULL on allocation failure.
 */
Structure *structure_create(Texture2D sprite, const char *name, bool passable,
                            bool lootable) {
  Structure *s = malloc(sizeof(Structure));
  if (!s)
    return NULL;
  s->sprite = sprite;
  s->passable = passable;
  s->lootable = lootable;
  if (name) {
    strncpy(s->name, name, sizeof(s->name) - 1);
    s->name[sizeof(s->name) - 1] = '\0';
  } else {
    s->name[0] = '\0';
  }
  return s;
}

/**
 * @brief Free a Structure previously created with structure_create.
 *
 * Does not free texture resources; those are managed by the renderer.
 */
void structure_free(Structure *s) {
  if (!s)
    return;
  free(s);
}
