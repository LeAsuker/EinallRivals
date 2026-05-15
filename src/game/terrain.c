#include "game/terrain.h"
#include "game/raylib_check.h"
#include "game/terrain_data.h"
#include <string.h>

/**
 * @brief Load an image at `path`, resize it to `cell_size`, and return a
 * texture.
 *
 * This helper wraps raylib Image load/resize and texture creation.
 */
static Texture2D load_terrain_texture(const char *path, int cell_size) {
  Image img = LoadImage(path);
  RAYLIB_CHECK_HAS_DATA(img);
  ImageResize(&img, cell_size, cell_size);
  Texture2D texture = LoadTextureFromImage(img);
  RAYLIB_CHECK_TEXTURE(texture);
  UnloadImage(img);
  return texture;
}

/**
 * @brief Initialize the Terrain array with predefined terrain types and
 * textures.
 *
 * Populates the `terrains` array using the provided `cell_size` for texture
 * scaling. Assumes `terrains` has at least enough slots for known terrain
 * enums.
 *
 * @param terrains Destination array of Terrain entries.
 * @param cell_size Size in pixels for each terrain sprite.
 */
void terrain_init_all(Terrain *terrains, int cell_size) {
  for (int i = 0; i < TERRAIN_COUNT; i++) {
    const TerrainMetadata *meta = terrain_data_get((TerrainType)i);
    terrains[i].id = meta->id;
    terrains[i].color = meta->color;
    terrains[i].passable = meta->passable;
    terrains[i].deep_version =
        (meta->deep_version < 0) ? NULL : &terrains[meta->deep_version];
    strcpy(terrains[i].name, meta->name);
    if (meta->texture_path != NULL) {
      terrains[i].sprite = load_terrain_texture(meta->texture_path, cell_size);
    }
  }
}

void terrain_unload_all(Terrain *terrains, int count) {
  for (int i = 0; i < count; i++) {
    if (terrains[i].sprite.id > 0) {
      UnloadTexture(terrains[i].sprite);
    }
  }
}
