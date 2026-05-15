#include "render/unit_sprites.h"
#include "render/structure_sprites.h"
#include "game/raylib_check.h"

/**
 * @brief Load an image from disk, resize it to cell_size, and convert to a
 * Texture2D.
 * @param path Filesystem path to the image file.
 * @param cell_size Size in pixels to resize the image to (square).
 * @return Loaded Texture2D (caller is responsible for unloading with
 * UnloadTexture).
 */
static Texture2D load_unit_texture(const char *path, int cell_size) {
  Image img = LoadImage(path);
  RAYLIB_CHECK_HAS_DATA(img);
  ImageResize(&img, cell_size, cell_size);
  Texture2D texture = LoadTextureFromImage(img);
  RAYLIB_CHECK_TEXTURE(texture);
  UnloadImage(img);
  return texture;
}

/**
 * @brief Load all unit textures scaled to the given cell size.
 * @param cell_size Target pixel size for each unit texture.
 * @return UnitSprites struct containing loaded textures (caller should call
 * unit_sprites_unload).
 */
UnitSprites unit_sprites_load(int cell_size) {
  UnitSprites sprites;
  sprites.darkus_militia =
      load_unit_texture("units/darkus_militia.png", cell_size);
  sprites.ventus_militia =
      load_unit_texture("units/ventus_militia.png", cell_size);
  sprites.darkus_spearman =
      load_unit_texture("units/darkus_spearman.png", cell_size);
  sprites.ventus_spearman =
      load_unit_texture("units/ventus_spearman.png", cell_size);
  sprites.darkus_swordsman =
      load_unit_texture("units/darkus_swordsman.png", cell_size);
  sprites.ventus_swordsman =
      load_unit_texture("units/ventus_swordsman.png", cell_size);
  sprites.warg = load_unit_texture("units/warg.png", cell_size);
  return sprites;
}

/**
 * @brief Unload textures contained in a UnitSprites struct.
 * @param sprites Pointer to UnitSprites whose textures will be unloaded
 * (NULL-safe behavior not required).
 */
void unit_sprites_unload(UnitSprites *sprites) {
  UnloadTexture(sprites->darkus_militia);
  UnloadTexture(sprites->ventus_militia);
  UnloadTexture(sprites->darkus_spearman);
  UnloadTexture(sprites->ventus_spearman);
  UnloadTexture(sprites->darkus_swordsman);
  UnloadTexture(sprites->ventus_swordsman);
  UnloadTexture(sprites->warg);
}

// Structure sprite helpers (kept in this compilation unit to include in build)
/**
 * @brief Load structure sprites scaled to the provided cell size.
 * @param cell_size Pixel size to scale structure textures to.
 * @return StructureSprites container with loaded textures.
 */
StructureSprites structure_sprites_load(int cell_size) {
  StructureSprites sprites;
  sprites.warg_lair =
      load_unit_texture("structures/warg_lair.png", cell_size);
  sprites.abandoned_hut = load_unit_texture(
      "structures/abandoned_hut.png", cell_size);
  return sprites;
}

/**
 * @brief Unload textures contained in a StructureSprites struct.
 * @param sprites Pointer to StructureSprites whose textures will be unloaded.
 */
void structure_sprites_unload(StructureSprites *sprites) {
  UnloadTexture(sprites->warg_lair);
}
