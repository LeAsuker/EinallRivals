#ifndef UNIT_SPRITES_H_
#define UNIT_SPRITES_H_

#include "raylib.h"

// Unit sprite container
typedef struct {
    Texture2D darkus_militia;
    Texture2D ventus_militia;
    Texture2D warg;
} UnitSprites;

// Load all unit sprites
UnitSprites unit_sprites_load(int cell_size);

// Unload all unit sprites
void unit_sprites_unload(UnitSprites *sprites);

#endif
