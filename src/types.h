#ifndef TYPES_H_
#define TYPES_H_

#include "raylib.h"
#include <stdbool.h>
struct Terrain;
typedef struct Terrain Terrain;

struct Terrain {
  int id;
  Color color;
  Texture2D sprite;
  bool passable;
  Terrain * deep_version;
  char name[10];
};

typedef struct Faction {
  Color prim_color;
  Color sec_color;
  bool has_turn;
  char name[10];
} Faction;

typedef struct Actor {
  Texture2D sprite;
  Faction *owner;
  bool can_move;
  bool can_act;

  int level;
  int next_level_xp;

  int max_health;
  int curr_health;

  int movement;

  int attack;
  int defense;

  int magic_defense;
  int magic_attack;

  int range;
  char name[10];
} Actor;

typedef struct Point {
  int x;
  int y;
  Actor *occupant;
  bool in_range;
  bool in_attack_range;
  Terrain terrain;
} Point;

typedef struct {
  Terrain terrain;
  int max_cores; // Maximum number of biome cores
  int max_range; // Maximum spread range
} BiomeConfig;

typedef struct {
  int grid_offset_x;
  int grid_offset_y;
  int grid_cell_size;
  int max_grid_cells_x;
  int max_grid_cells_y;
} GridConfig;

#endif