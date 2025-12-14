#include "unit_sprites.h"

// Helper to load and resize unit textures
static Texture2D load_unit_texture(const char *path, int cell_size) {
    Image img = LoadImage(path);
    ImageResize(&img, cell_size, cell_size);
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

UnitSprites unit_sprites_load(int cell_size) {
    UnitSprites sprites;
    sprites.darkus_militia = load_unit_texture("../../resources/darkus_militia.png", cell_size);
    sprites.ventus_militia = load_unit_texture("../../resources/ventus_militia.png", cell_size);
    return sprites;
}

void unit_sprites_unload(UnitSprites *sprites) {
    UnloadTexture(sprites->darkus_militia);
    UnloadTexture(sprites->ventus_militia);
}