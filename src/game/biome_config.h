#ifndef BIOME_CONFIG_H_
#define BIOME_CONFIG_H_

#include "types.h"

/**
 * @brief Populate an array with a set of default BiomeConfig entries.
 *
 * Fills at most max_configs entries from a predefined set (Arctic, Forest,
 * Sea). Each BiomeConfig references a Terrain entry from the provided
 * `terrains` array.
 *
 * @param configs Preallocated array to receive biome configurations.
 * @param max_configs Maximum number of entries available in `configs`.
 * @param terrains Array of Terrain entries indexed by Terrain enum.
 * @return Number of biome configs written to `configs`.
 */
int biome_config_get_default(BiomeConfig *configs, int max_configs, Terrain *terrains);

#endif
