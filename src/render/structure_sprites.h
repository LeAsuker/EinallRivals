#ifndef STRUCTURE_SPRITES_H_
#define STRUCTURE_SPRITES_H_

#include "raylib.h"

/**
 * @brief Container of preloaded structure textures.
 */
typedef struct {
    Texture2D warg_lair;      /**< Texture for a warg lair. */
    Texture2D abandoned_hut;  /**< Texture for an abandoned hut. */
} StructureSprites;

/**
 * @brief Load structure sprites scaled to the provided cell size.
 * @param cell_size Pixel size to scale structure textures to.
 * @return StructureSprites container with loaded textures.
 */
StructureSprites structure_sprites_load(int cell_size);

/**
 * @brief Unload textures contained in a StructureSprites struct.
 * @param sprites Pointer to StructureSprites whose textures will be unloaded.
 */
void structure_sprites_unload(StructureSprites *sprites);

#endif
