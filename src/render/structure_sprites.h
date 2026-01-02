#ifndef STRUCTURE_SPRITES_H_
#define STRUCTURE_SPRITES_H_

#include "raylib.h"

typedef struct {
    Texture2D warg_lair;
} StructureSprites;

StructureSprites structure_sprites_load(int cell_size);
void structure_sprites_unload(StructureSprites *sprites);

#endif
