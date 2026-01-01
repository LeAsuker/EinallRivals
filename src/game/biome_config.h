#ifndef BIOME_CONFIG_H_
#define BIOME_CONFIG_H_

#include "types.h"

// Creates default biome configurations for world generation
// Returns the number of biomes configured
int biome_config_get_default(BiomeConfig *configs, int max_configs, Terrain *terrains);

#endif
