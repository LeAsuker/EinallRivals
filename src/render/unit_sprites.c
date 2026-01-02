#include "render/unit_sprites.h"
#include "render/structure_sprites.h"

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
    sprites.darkus_militia = load_unit_texture("../../resources/units/darkus_militia.png", cell_size);
    sprites.ventus_militia = load_unit_texture("../../resources/units/ventus_militia.png", cell_size);
    sprites.warg = load_unit_texture("../../resources/units/warg.png", cell_size);
    return sprites;
}

void unit_sprites_unload(UnitSprites *sprites) {
    UnloadTexture(sprites->darkus_militia);
    UnloadTexture(sprites->ventus_militia);
    UnloadTexture(sprites->warg);
}

// Structure sprite helpers (kept in this compilation unit to include in build)
StructureSprites structure_sprites_load(int cell_size) {
    StructureSprites sprites;
    sprites.warg_lair = load_unit_texture("../../resources/structures/warg_lair.png", cell_size);
    return sprites;
}

void structure_sprites_unload(StructureSprites *sprites) {
    UnloadTexture(sprites->warg_lair);
}
