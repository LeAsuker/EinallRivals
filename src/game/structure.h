#ifndef STRUCTURE_H_
#define STRUCTURE_H_

#include "types.h"
#include <stdbool.h>

Structure *structure_create(Texture2D sprite, const char *name, bool passable, bool lootable);
void structure_free(Structure *s);

#endif
