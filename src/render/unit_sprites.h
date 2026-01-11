#ifndef UNIT_SPRITES_H_
#define UNIT_SPRITES_H_

#include "raylib.h"

/**
 * @brief Container holding textures for different unit types.
 */
typedef struct {
    Texture2D darkus_militia;
    Texture2D ventus_militia;
    Texture2D warg;
} UnitSprites;

/**
 * @brief Load unit textures scaled to `cell_size` and return a populated UnitSprites.
 */
UnitSprites unit_sprites_load(int cell_size);

/**
 * @brief Unload textures contained in `sprites`.
 */
void unit_sprites_unload(UnitSprites *sprites);

#endif
