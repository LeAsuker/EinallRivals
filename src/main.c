/*******************************************************************************************
*
*   raylib [core] example - undo redo
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: calloc(), free()
#include <string.h>     // Required for: memcpy(), strcmp()
#include "stdio.h"

#define GRID_CELL_SIZE              24      
#define MAX_GRID_CELLS_X            30
#define MAX_GRID_CELLS_Y            13

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Point struct, like Vector2 but using int

// Player state struct
// NOTE: Contains all player data that needs to be affected by undo/redo

struct Point;
struct PlayerState;
struct Terrain;

typedef struct Point Point;
typedef struct PlayerState PlayerState;
typedef struct Terrain Terrain;

struct Terrain {
    Color color;
};

struct Point {
    int x;
    int y;
    PlayerState* occupant;
    bool selected;
    Terrain terrain;
};


struct PlayerState {
    Color color;
    bool selected;
};

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------

// is offset
Point * mouseToCell(Vector2 gridPosition, Point * point_arr) {
    Point cell;
    cell.x = (GetMouseX() - gridPosition.x) / GRID_CELL_SIZE;
    cell.y = (GetMouseY() - gridPosition.y) / GRID_CELL_SIZE;
    Point* cell_in_map = point_arr + MAX_GRID_CELLS_X*cell.y + cell.x;
    return cell_in_map;
}

// if mouse in given cell
bool mouseInCell(Vector2 gridPosition, Point cell) {
    if (GetMouseX() <= gridPosition.x + cell.x*GRID_CELL_SIZE + GRID_CELL_SIZE){
        if (GetMouseY() <= gridPosition.y + cell.y*GRID_CELL_SIZE + GRID_CELL_SIZE) {
            if (GetMouseX() >= gridPosition.x + cell.x*GRID_CELL_SIZE){
                if (GetMouseY() >= gridPosition.y + cell.y*GRID_CELL_SIZE) {
                    return true;
                }
            }
        }
    }
    return false;
}

void actor_deselect(PlayerState * actor) {
    actor->selected = false;
    actor->color.r = 0;
    actor->color.g = 0;
    actor->color.b = 255;
    return;
}


void actor_select(PlayerState * actor) {
    actor->selected = true;
    actor->color.r = 255;
    actor->color.g = 255;
    actor->color.b = 0;
    return;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "WaterEmblemProto");
    
    
    // Grid variables
    Vector2 gridPosition = { 40, 60 };
    
    // map init
    Point* mapArr = malloc(sizeof(Point)*MAX_GRID_CELLS_X*MAX_GRID_CELLS_Y);
    for (int yCoor = 0; yCoor < MAX_GRID_CELLS_Y; yCoor++) {
        for (int xCoor = 0; xCoor < MAX_GRID_CELLS_X; xCoor++) {
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].x = xCoor;
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].y = yCoor;
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].occupant = NULL;
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].selected = false;
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].terrain.color = GREEN;

        }
    }
    // Init current player state
    PlayerState player;
    player.color = BLUE;
    player.selected = false;

    mapArr[100].occupant = &player;
    Point * last_player_position = mapArr + 100;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Make sure player does not go out of bounds
        // if (player.cell.x < 0) player.cell.x = 0;
        // else if (player.cell.x >= MAX_GRID_CELLS_X) player.cell.x = MAX_GRID_CELLS_X - 1;
        // if (player.cell.y < 0) player.cell.y = 0;
        // else if (player.cell.y >= MAX_GRID_CELLS_Y) player.cell.y = MAX_GRID_CELLS_Y - 1;

        // The XY coords are in the top left corner of the square
        if (IsMouseButtonPressed(0)) {
            Point * selected_cell = mouseToCell(gridPosition, mapArr);
            printf("selected_cell %d %d", selected_cell->x, selected_cell->y);
            // tells us which cell we have selected in the mapArr
            // had to do pointer stuff to point to the permanent object


            if (selected_cell->selected) {
                selected_cell->selected = false;
            }
            else {
                selected_cell->selected = true;
            }
            
            if (player.selected && !(selected_cell->occupant == &player)){
                printf("move \n");
                selected_cell->occupant = &player;
                last_player_position->occupant = NULL;
                last_player_position = selected_cell;
                
                actor_deselect(&player);
            }
            else if (selected_cell->occupant == &player){
                if (player.selected) {
                    actor_deselect(&player);
                }
                else {
                    actor_select(&player);
                }
            }
            
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw debug info
            DrawText(TextFormat("MOUSE: %d %d - MCELL: %d %d", GetMouseX(), GetMouseY(),
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
                }

                DrawRectangleLines(cell_x_pos, cell_y_pos,
                    GRID_CELL_SIZE, GRID_CELL_SIZE, GRAY);

                if (mapArr[cellIdx].selected) {
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