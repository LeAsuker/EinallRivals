#include "raylib.h"

#include <stdlib.h>     // Required for: calloc(), free()
#include <string.h>     // Required for: memcpy(), strcmp()
#include <stdio.h>
#include <time.h>

#include "main.h"

#define GRID_CELL_SIZE              30      
#define MAX_GRID_CELLS_X            30
#define MAX_GRID_CELLS_Y            21

int main(void)
{
    // Define terrain types
    Terrain Plains = { .color = GREEN };
    Terrain Mountains = { .color = LIGHTGRAY };
    Terrain Sea = { .color = DARKBLUE };
    Terrain Arctic = { .color = WHITE };
    Terrain Forest = { .color = DARKGREEN };
    
    // Configure each biome type
    BiomeConfig biome_configs[] = {
        { .terrain = Mountains, .max_cores = 3, .max_range = 3 },
        { .terrain = Arctic, .max_cores = 3, .max_range = 3 },
        { .terrain = Forest, .max_cores = 3, .max_range = 3 },
        { .terrain = Sea,    .max_cores = 2, .max_range = 5 }
    };
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;
    srand(time(NULL));
    
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
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].in_range = false;
            mapArr[xCoor + yCoor*MAX_GRID_CELLS_X].terrain = Plains;
            
        }
    }
    int num_biomes = sizeof(biome_configs) / sizeof(BiomeConfig);
    int layers = 7;
    generate_all_biomes(mapArr, biome_configs, num_biomes, layers);

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
    Actor player;
    actor_init(&player, &Azai);
    
    Point * spawn = get_random_cell(mapArr);
    spawn->occupant = &player;
    Point * last_player_position = spawn;
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // The XY coords are in the top left corner of the square
        if (IsMouseButtonPressed(0)) {
            Point * selected_cell = mouseToCell(gridPosition, mapArr);
            printf("selected_cell %d %d", selected_cell->x, selected_cell->y);
            // tells us which cell we have selected in the mapArr
            // had to do pointer stuff to point to the permanent object
            
            
            if (player.selected && !(selected_cell->occupant == &player) && selected_cell->in_range){
                printf("move \n");
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
        
        // Draw debug info
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
                Actor* occupant = curr_cell.occupant;
                DrawRectangle(cell_x_pos, cell_y_pos,
                    GRID_CELL_SIZE, GRID_CELL_SIZE, occupant->color);
                DrawRectangle(cell_x_pos, cell_y_pos,
                    GRID_CELL_SIZE - 10, GRID_CELL_SIZE - 10, occupant->owner->prim_color);
                
                if (occupant->selected){
                    DrawText(TextFormat("LVL: %d\nEXP NEEDED: %d\nHP: %d/%d\nMOV: %d\nATK: %d\nDEF: %d\n",
                        occupant->level, occupant->next_level_xp, occupant->curr_health, occupant->max_health,
                        occupant->movement, occupant->attack, occupant->defense),
                        MAX_GRID_CELLS_X*GRID_CELL_SIZE + gridPosition.x + 20, gridPosition.y, 20, BLACK);
                    }
            }
                
            DrawRectangleLines(cell_x_pos, cell_y_pos,
                GRID_CELL_SIZE, GRID_CELL_SIZE, GRAY);
                        
            if (curr_cell.in_range == true) {
                DrawRectangleLines(cell_x_pos, cell_y_pos,
                    GRID_CELL_SIZE, GRID_CELL_SIZE, RED);
            }
        }

        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(mapArr);
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}


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
    // quick fix, also removes targeting and affects flyers
    // comparison problem, will have to refactor this
    if (start_cell->terrain.color.b == DARKBLUE.b) {
        return;
    }
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

void spread_terrain(Point* cell_arr, Point* start_cell, int range, Terrain terrain) {
    // Set current cell's terrain
    start_cell->terrain = terrain;
    
    // Base case: stop spreading
    if (range == 0) {
        return;
    }
    
    int x = start_cell->x;
    int y = start_cell->y;
    int x_limit = MAX_GRID_CELLS_X - 1;
    int y_limit = MAX_GRID_CELLS_Y - 1;
    
    // Spread to adjacent cells (up, down, left, right)
    if (y != 0) {
        Point* cell_up = cell_arr + x + MAX_GRID_CELLS_X * (y - 1);
        spread_terrain(cell_arr, cell_up, range - 1, terrain);
    }
    if (y < y_limit) {
        Point* cell_down = cell_arr + x + MAX_GRID_CELLS_X * (y + 1);
        spread_terrain(cell_arr, cell_down, range - 1, terrain);
    }
    if (x != 0) {
        Point* cell_left = cell_arr + x - 1 + MAX_GRID_CELLS_X * y;
        spread_terrain(cell_arr, cell_left, range - 1, terrain);
    }
    if (x < x_limit) {
        Point* cell_right = cell_arr + x + 1 + MAX_GRID_CELLS_X * y;
        spread_terrain(cell_arr, cell_right, range - 1, terrain);
    }
}

Point* get_random_cell(Point* cell_arr) {
    int rand_x = rand() % MAX_GRID_CELLS_X;
    int rand_y = rand() % MAX_GRID_CELLS_Y;
    return cell_arr + rand_x + rand_y * MAX_GRID_CELLS_X;
}

void generate_biome_cores(Point* cell_arr, BiomeConfig config) {
    // since modulo is offset
    int num_cores = rand() % (config.max_cores + 1);
    
    for (int i = 0; i < num_cores; i++) {
        Point* core = get_random_cell(cell_arr);
        int range = (rand() % config.max_range) + 1;
        spread_terrain(cell_arr, core, range, config.terrain);
    }
}

void generate_all_biomes(Point* cell_arr, BiomeConfig* biome_configs, int num_biomes, int layers) {
    for (int layer = 0; layer < layers; layer++) {
        for (int i = 0; i < num_biomes; i++) {
            generate_biome_cores(cell_arr, biome_configs[i]);
        }
    }
}

void actor_init( Actor * actor, Player * owner) {
    actor->color = owner->sec_color;
    actor->og_color = owner->sec_color;
    actor->owner = owner;
    actor->selected = false;

    actor->level = 1;
    actor->next_level_xp = 100;

    actor->max_health = 20;
    actor->curr_health = 20;

    actor->movement = 4;

    actor->attack = 8;
    actor->defense = 3;

    actor->magic_defense = 3;
    actor->magic_attack = 2;

    actor->range = 1;
}