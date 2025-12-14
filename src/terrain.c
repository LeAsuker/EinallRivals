#include "terrain.h"
#include <string.h>

// Helper to load and resize textures
static Texture2D load_terrain_texture(const char *path, int cell_size) {
    Image img = LoadImage(path);
    ImageResize(&img, cell_size, cell_size);
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

void terrain_init_all(Terrain *terrains, int cell_size) {
    // Initialize None terrain (no texture)
    terrains[TERRAIN_NONE].id = -1;
    terrains[TERRAIN_NONE].color = WHITE;
    terrains[TERRAIN_NONE].passable = false;
    terrains[TERRAIN_NONE].deep_version = NULL;
    strcpy(terrains[TERRAIN_NONE].name, "None");
    
    // Initialize DeepForest
    terrains[TERRAIN_DEEP_FOREST].id = 41;
    terrains[TERRAIN_DEEP_FOREST].color = BLACK;
    terrains[TERRAIN_DEEP_FOREST].passable = false;
    terrains[TERRAIN_DEEP_FOREST].deep_version = &terrains[TERRAIN_NONE];
    terrains[TERRAIN_DEEP_FOREST].sprite = load_terrain_texture("../../resources/deep_forest_ter.png", cell_size);
    strcpy(terrains[TERRAIN_DEEP_FOREST].name, "DeepForest");
    
    // Initialize DeepSea
    terrains[TERRAIN_DEEP_SEA].id = 21;
    terrains[TERRAIN_DEEP_SEA].color = DARKBLUE;
    terrains[TERRAIN_DEEP_SEA].passable = false;
    terrains[TERRAIN_DEEP_SEA].deep_version = &terrains[TERRAIN_NONE];
    terrains[TERRAIN_DEEP_SEA].sprite = load_terrain_texture("../../resources/deep_sea_ter.png", cell_size);
    strcpy(terrains[TERRAIN_DEEP_SEA].name, "DeepSea");
    
    // Initialize Plains
    terrains[TERRAIN_PLAINS].id = 0;
    terrains[TERRAIN_PLAINS].color = GREEN;
    terrains[TERRAIN_PLAINS].passable = true;
    terrains[TERRAIN_PLAINS].deep_version = &terrains[TERRAIN_NONE];
    terrains[TERRAIN_PLAINS].sprite = load_terrain_texture("../../resources/plains_ter.png", cell_size);
    strcpy(terrains[TERRAIN_PLAINS].name, "Plains");
    
    // Initialize Mountains
    terrains[TERRAIN_MOUNTAINS].id = 1;
    terrains[TERRAIN_MOUNTAINS].color = LIGHTGRAY;
    terrains[TERRAIN_MOUNTAINS].passable = false;
    terrains[TERRAIN_MOUNTAINS].deep_version = &terrains[TERRAIN_NONE];
    terrains[TERRAIN_MOUNTAINS].sprite = load_terrain_texture("../../resources/mountain_ter2.png", cell_size);
    strcpy(terrains[TERRAIN_MOUNTAINS].name, "Mountains");
    
    // Initialize Sea
    terrains[TERRAIN_SEA].id = 2;
    terrains[TERRAIN_SEA].color = BLUE;
    terrains[TERRAIN_SEA].passable = false;
    terrains[TERRAIN_SEA].deep_version = &terrains[TERRAIN_DEEP_SEA];
    terrains[TERRAIN_SEA].sprite = load_terrain_texture("../../resources/sea_ter.png", cell_size);
    strcpy(terrains[TERRAIN_SEA].name, "Sea");
    
    // Initialize Arctic/Hills
    terrains[TERRAIN_ARCTIC].id = 3;
    terrains[TERRAIN_ARCTIC].color = WHITE;
    terrains[TERRAIN_ARCTIC].passable = true;
    terrains[TERRAIN_ARCTIC].deep_version = &terrains[TERRAIN_MOUNTAINS];
    terrains[TERRAIN_ARCTIC].sprite = load_terrain_texture("../../resources/arctic_ter.png", cell_size);
    strcpy(terrains[TERRAIN_ARCTIC].name, "Hills");
    
    // Initialize Forest
    terrains[TERRAIN_FOREST].id = 4;
    terrains[TERRAIN_FOREST].color = DARKGREEN;
    terrains[TERRAIN_FOREST].passable = true;
    terrains[TERRAIN_FOREST].deep_version = &terrains[TERRAIN_DEEP_FOREST];
    terrains[TERRAIN_FOREST].sprite = load_terrain_texture("../../resources/forest_ter2.png", cell_size);
    strcpy(terrains[TERRAIN_FOREST].name, "Forest");
    
    // Initialize Player Base
    terrains[TERRAIN_PLAYER_BASE].id = 6;
    terrains[TERRAIN_PLAYER_BASE].color = ORANGE;
    terrains[TERRAIN_PLAYER_BASE].passable = true;
    terrains[TERRAIN_PLAYER_BASE].deep_version = &terrains[TERRAIN_NONE];
    terrains[TERRAIN_PLAYER_BASE].sprite = load_terrain_texture("../../resources/base_ter.png", cell_size);
    strcpy(terrains[TERRAIN_PLAYER_BASE].name, "Base");
}

void terrain_unload_all(Terrain *terrains, int count) {
    for (int i = 0; i < count; i++) {
        if (terrains[i].sprite.id > 0) {
            UnloadTexture(terrains[i].sprite);
        }
    }
}