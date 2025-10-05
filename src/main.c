#include "raylib.h"

#include <stdlib.h>     // Required for: calloc(), free()
#include <string.h>     // Required for: memcpy(), strcmp()
#include "stdio.h"
#include "time.h"

#define GRID_CELL_SIZE              30      
#define MAX_GRID_CELLS_X            25
#define MAX_GRID_CELLS_Y            18

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Point struct, like Vector2 but using int

// Player state struct
// NOTE: Contains all player data that needs to be affected by undo/redo

struct Point;
struct PlayerState;
struct Terrain;
struct Player;

typedef struct Point Point;
typedef struct PlayerState PlayerState;
typedef struct Terrain Terrain;
typedef struct Player Player;

// make an arr of types of these
// and make each cell point to its type
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

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
// prevents out of bounds and segfaults
// maybe unify these two functions?

int safe_mouse_x(Vector2 gridPosition);
int safe_mouse_y(Vector2 gridPosition);
Point * mouseToCell(Vector2 gridPosition, Point * point_arr);
bool mouseInCell(Vector2 gridPosition, Point cell);
void actor_selection(Point * cell_arr, Point * cell);
void range_calc(Point * cell_arr, Point * start_cell, int range, bool selection);

int safe_mouse_x(Vector2 gridPosition) {
    int mouse_pos = GetMouseX();
    if ( mouse_pos >= gridPosition.x + MAX_GRID_CELLS_X*GRID_CELL_SIZE ) {
        return gridPosition.x + MAX_GRID_CELLS_X*GRID_CELL_SIZE - 1;
    } 
    else if (mouse_pos <= gridPosition.x) {
        return gridPosition.x + 1;
    }
    return mouse_pos;
}

int safe_mouse_y(Vector2 gridPosition) {
    int mouse_pos = GetMouseY();
    if ( mouse_pos >= gridPosition.y + MAX_GRID_CELLS_Y*GRID_CELL_SIZE ) {
        return gridPosition.y + MAX_GRID_CELLS_Y*GRID_CELL_SIZE - 1;
    } 
    else if (mouse_pos <= gridPosition.y) {
        return gridPosition.y + 1;
    }
    return mouse_pos;
}

Point * mouseToCell(Vector2 gridPosition, Point * point_arr) {
    // will need to sanitize it so it cant ever go out of bounds, else undefined
    int x = (safe_mouse_x(gridPosition) - gridPosition.x) / GRID_CELL_SIZE;
    int y = (safe_mouse_y(gridPosition) - gridPosition.y) / GRID_CELL_SIZE;

    Point* cell_in_map = point_arr + MAX_GRID_CELLS_X*y + x;
    return cell_in_map;
}

// if mouse in given cell
bool mouseInCell(Vector2 gridPosition, Point cell) {
    if (safe_mouse_x(gridPosition) <= gridPosition.x + cell.x*GRID_CELL_SIZE + GRID_CELL_SIZE){
        if (safe_mouse_y(gridPosition) <= gridPosition.y + cell.y*GRID_CELL_SIZE + GRID_CELL_SIZE) {
            if (safe_mouse_x(gridPosition) >= gridPosition.x + cell.x*GRID_CELL_SIZE){
                if (safe_mouse_y(gridPosition) >= gridPosition.y + cell.y*GRID_CELL_SIZE) {
                    return true;
                }
            }
        }
    }
    return false;
}
// could be issues here with occupant NULL checks...
// currently depends on in_range prev position flag
// removal
void actor_selection(Point * cell_arr, Point * cell) {

    if (cell->occupant->selected == true) {
        cell->occupant->selected = false;
        cell->occupant->color = cell->occupant->owner->sec_color;
        range_calc(cell_arr, cell, cell->occupant->movement, false);
    }
    
    else {
        cell->occupant->selected = true;
        cell->occupant->color = YELLOW;
        range_calc(cell_arr, cell, cell->occupant->movement, true);
    }
    return;
}

void range_calc(Point * cell_arr, Point * start_cell, int range, bool selection) {
    // base case
    start_cell->in_range = selection;
    if (range == 0) {
        return;
    }
    int x = start_cell->x;
    int y = start_cell->y;
    int x_limit = MAX_GRID_CELLS_X - 1;
    int y_limit = MAX_GRID_CELLS_Y - 1;
    if (y != 0) {
        Point * cell_up = cell_arr + x + MAX_GRID_CELLS_X*(y - 1);
        range_calc(cell_arr, cell_up, range - 1, selection);

    }
    if (y < y_limit) {

        Point * cell_down = cell_arr + x + MAX_GRID_CELLS_X*(y + 1);
        range_calc(cell_arr, cell_down, range - 1, selection);
    }
    if (x != 0) {
        Point * cell_left = cell_arr + x - 1 + MAX_GRID_CELLS_X*(y);
        range_calc(cell_arr, cell_left, range - 1, selection);
    }
    if (x < x_limit) {
        Point * cell_right = cell_arr + x + 1 + MAX_GRID_CELLS_X*(y);
        range_calc(cell_arr, cell_right, range - 1, selection);
    }

    return;
}

void biome_generator(Point* cell_arr, Point * start_cell, int range, Terrain terrain) {
    start_cell->terrain = terrain;
    if (range == 0) {
        return;
    }
    int x = start_cell->x;
    int y = start_cell->y;
    int x_limit = MAX_GRID_CELLS_X - 1;
    int y_limit = MAX_GRID_CELLS_Y - 1;
    if (y != 0) {
        Point * cell_up = cell_arr + x + MAX_GRID_CELLS_X*(y - 1);
        biome_generator(cell_arr, cell_up, range - 1, terrain);

    }
    if (y < y_limit) {

        Point * cell_down = cell_arr + x + MAX_GRID_CELLS_X*(y + 1);
        biome_generator(cell_arr, cell_down, range - 1, terrain);
    }
    if (x != 0) {
        Point * cell_left = cell_arr + x - 1 + MAX_GRID_CELLS_X*(y);
        biome_generator(cell_arr, cell_left, range - 1, terrain);
    }
    if (x < x_limit) {
        Point * cell_right = cell_arr + x + 1 + MAX_GRID_CELLS_X*(y);
        biome_generator(cell_arr, cell_right, range - 1, terrain);
    }

    return;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;
    srand(time(NULL));
    
    InitWindow(screenWidth, screenHeight, "WaterEmblemProto");
    
    
    // Grid variables
    Vector2 gridPosition = { 60, 90 };
    
    // map init
    Point* mapArr = malloc(sizeof(Point)*MAX_GRID_CELLS_X*MAX_GRID_CELLS_Y);
    for (int yCoor = 0; yCoor < MAX_GRID_CELLS_Y; yCoor++) {
        for (int xCoor = 0; xCoor < MAX_GRID_CELLS_X; xCoor++) {
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].x = xCoor;
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].y = yCoor;
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].occupant = NULL;
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].in_range = false;
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].terrain.color = GREEN;
            
        }
    }
    Terrain Plains;
    Plains.color = GREEN;

    Terrain Desert;
    Desert.color = YELLOW;

    Terrain Sea;
    Sea.color = DARKBLUE;

    Terrain Arctic;
    Arctic.color = WHITE;
    
    Terrain Forest;
    Forest.color = DARKGREEN;

    // biome painting
    for (int i = 0; i < 2; i++) {
        int desert_num = rand() % 5;
        for (int d = 0; d < desert_num; d++) {
            int rand_x = rand() % MAX_GRID_CELLS_X;
            int rand_y = rand() % MAX_GRID_CELLS_Y;
            Point* desert_core =  mapArr + rand_x + rand_y*MAX_GRID_CELLS_X;
            biome_generator(mapArr, desert_core, (rand() % 5) + 1, Desert); 
        }

        
        int arctic_num = rand() % 6;
        for (int a = 0; a < arctic_num; a++) {
            int rand_x = rand() % MAX_GRID_CELLS_X;
            int rand_y = rand() % MAX_GRID_CELLS_Y;
            Point* arctic_core =  mapArr + rand_x + rand_y*MAX_GRID_CELLS_X;
            biome_generator(mapArr, arctic_core, (rand() % 6) + 1, Arctic); 
        }
        
        int forest_num = rand() % 3;
        for (int f = 0; f < forest_num; f++) {
            int rand_x = rand() % MAX_GRID_CELLS_X;
            int rand_y = rand() % MAX_GRID_CELLS_Y;
            Point* forest_core =  mapArr + rand_x + rand_y*MAX_GRID_CELLS_X;
            biome_generator(mapArr, forest_core, (rand() % 3) + 1, Forest); 
        }
        int sea_num = rand() % 7;
        for (int s = 0; s < sea_num; s++) {
            int rand_x = rand() % MAX_GRID_CELLS_X;
            int rand_y = rand() % MAX_GRID_CELLS_Y;
            Point* sea_core =  mapArr + rand_x + rand_y*MAX_GRID_CELLS_X;
            biome_generator(mapArr, sea_core, (rand() % 7) + 1, Sea); 
        }
    }
    // init faction player
    Player Azai;
    Azai.prim_color = PURPLE;
    Azai.sec_color = DARKGRAY;
    Azai.has_turn = true;

    Player Anegakoji;
    Anegakoji.prim_color = GREEN;
    Anegakoji.sec_color = WHITE;
    Anegakoji.has_turn = true;

    Player Gaia;
    Gaia.prim_color = BROWN;
    Gaia.sec_color = BLACK;
    Gaia.has_turn = true;


    // Init current player state
    PlayerState player;
    player.owner = &Azai;
    player.color = player.owner->sec_color;
    player.selected = false;
    player.movement = 7;
    player.max_health = 20;
    player.curr_health = 20;
    player.attack = 8;
    player.armor = 3;

    mapArr[100].occupant = &player;
    Point * last_player_position = mapArr + 100;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // The XY coords are in the top left corner of the square
        // mouse in bounds does not work due to undefined behavior,
        // am accessing elements outside of the cell array
        if (IsMouseButtonPressed(0)) {
            Point * selected_cell = mouseToCell(gridPosition, mapArr);
            printf("selected_cell %d %d", selected_cell->x, selected_cell->y);
            // tells us which cell we have selected in the mapArr
            // had to do pointer stuff to point to the permanent object


            if (player.selected && !(selected_cell->occupant == &player) && selected_cell->in_range){
                printf("move \n");
                // replace 3 with player movement
                // this removes the in_range flag to the tiles around original position
                range_calc(mapArr, last_player_position, player.movement, false);
                selected_cell->occupant = &player;
                last_player_position->occupant = NULL;
                last_player_position = selected_cell;
                
                actor_selection(mapArr, selected_cell);
            }
            else if (selected_cell->occupant == &player){
                actor_selection(mapArr, selected_cell);
            }
            
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw debug info, this is broken with undefined behavior
            DrawText(TextFormat("MOUSE: %d %d - MCELL: %d %d", safe_mouse_x(gridPosition), safe_mouse_y(gridPosition),
                mouseToCell(gridPosition, mapArr)->x, mouseToCell(gridPosition, mapArr)->y), 40, 20, 20, DARKGRAY);
            
            // draws game map through mapArr array
            for (int cellIdx = 0; cellIdx < MAX_GRID_CELLS_X*MAX_GRID_CELLS_Y; cellIdx++) {

                int cell_x_pos = gridPosition.x + mapArr[cellIdx].x*GRID_CELL_SIZE;
                int cell_y_pos = gridPosition.y + mapArr[cellIdx].y*GRID_CELL_SIZE;
                Point curr_cell = mapArr[cellIdx];

                // first we draw terrain, on it occupant, then grid, then selection
                DrawRectangle(cell_x_pos, cell_y_pos,
                    GRID_CELL_SIZE, GRID_CELL_SIZE, curr_cell.terrain.color);
                
                    
                if (curr_cell.occupant != NULL) {
                    DrawRectangle(cell_x_pos, cell_y_pos,
                        GRID_CELL_SIZE, GRID_CELL_SIZE, curr_cell.occupant->color);
                    DrawRectangle(cell_x_pos, cell_y_pos,
                        GRID_CELL_SIZE - 10, GRID_CELL_SIZE - 10, curr_cell.occupant->owner->prim_color);
                }
                    
                DrawRectangleLines(cell_x_pos, cell_y_pos,
                    GRID_CELL_SIZE, GRID_CELL_SIZE, GRAY);
                            
                if (curr_cell.in_range == true) {
                    DrawRectangleLines(cell_x_pos, cell_y_pos,
                        GRID_CELL_SIZE, GRID_CELL_SIZE, RED);
                }
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(mapArr);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}