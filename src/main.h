#ifndef MAIN_H_
#define MAIN_H_
struct Terrain {
    Color color;
};

struct Point {
    int x;
    int y;
    PlayerState* occupant;
    bool in_range;
    Terrain terrain;
};


struct PlayerState {
    Color color;
    Color og_color;
    Player * owner;
    bool selected;

    int max_health;
    int curr_health;
    int movement;
    int attack;
    int armor;
};

struct Player {
    Color prim_color;
    Color sec_color;
    bool has_turn;
};

typedef struct Point Point;
typedef struct PlayerState PlayerState;
typedef struct Terrain Terrain;
typedef struct Player Player;

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
#endif