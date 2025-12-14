#include "biome_config.h"
#include "terrain.h"

int biome_config_get_default(BiomeConfig *configs, int max_configs, Terrain *terrains) {
    if (max_configs < 3) {
        return 0;
    }
    
    // Arctic/Hills biome
    configs[0].terrain = terrains[TERRAIN_ARCTIC];
    configs[0].max_cores = 3;
    configs[0].max_range = 4;
    
    // Forest biome
    configs[1].terrain = terrains[TERRAIN_FOREST];
    configs[1].max_cores = 5;
    configs[1].max_range = 4;
    
    // Sea biome
    configs[2].terrain = terrains[TERRAIN_SEA];
    configs[2].max_cores = 2;
    configs[2].max_range = 5;
    
    return 3;
}