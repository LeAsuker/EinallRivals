#ifndef MAIN_H_
#define MAIN_H_
typedef struct Terrain {
    Color color;
} Terrain;

typedef struct Player {
    Color prim_color;
    Color sec_color;
    bool has_turn;
} Player;

typedef struct PlayerState {
    Color color;
    Color og_color;
    Player * owner;
    bool selected;

    int max_health;
    int curr_health;
    int movement;
    int attack;
    int armor;
} PlayerState;

typedef struct Point {
    int x;
    int y;
    PlayerState* occupant;
    bool in_range;
    Terrain terrain;
} Point;

typedef struct {
    Terrain terrain;
    int max_cores;      // Maximum number of biome cores
    int max_range;      // Maximum spread range
} BiomeConfig;

int safe_mouse_x(Vector2 gridPosition);
int safe_mouse_y(Vector2 gridPosition);
Point * mouseToCell(Vector2 gridPosition, Point * point_arr);
bool mouseInCell(Vector2 gridPosition, Point cell);
void actor_selection(Point * cell_arr, Point * cell);
void range_calc(Point * cell_arr, Point * start_cell, int range, bool selection);
void spread_terrain(Point* cell_arr, Point* start_cell, int range, Terrain terrain);
Point* get_random_cell(Point* cell_arr);
void generate_biome_cores(Point* cell_arr, BiomeConfig config);
void generate_all_biomes(Point* cell_arr, BiomeConfig* biome_configs, int num_biomes, int layers);
#endif