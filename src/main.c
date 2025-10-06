#include "raylib.h"

#include <stdlib.h>     // Required for: calloc(), free()
#include <string.h>     // Required for: memcpy(), strcmp()
#include <stdio.h>
#include <time.h>

#include "main.h"

#define GRID_CELL_SIZE              30      
#define MAX_GRID_CELLS_X            30
#define MAX_GRID_CELLS_Y            21
#define DARK_TROOP_NUM               6
#define VENT_TROOP_NUM               6

int main(void)
{
    Image sea_sprite = LoadImage("resources/sea_ter.png");
    Image mountains_sprite = LoadImage("resources/mountain_ter.png");
    Image plains_sprite = LoadImage("resources/plains_ter.png");
    Image arctic_sprite = LoadImage("resources/arctic_ter.png");
    Image forest_sprite = LoadImage("resources/forest_ter.png");

    Image v_militia_sprite = LoadImage("resources/ventus_militia.png");
    Image d_militia_sprite = LoadImage("resources/darkus_militia.png");

    ImageResize(&sea_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
    ImageResize(&mountains_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
    ImageResize(&plains_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
    ImageResize(&arctic_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
    ImageResize(&forest_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);

    ImageResize(&d_militia_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);
    ImageResize(&v_militia_sprite, GRID_CELL_SIZE, GRID_CELL_SIZE);

    // Define terrain types
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;
    srand(time(NULL));
    
    // initwindow creates opengl context, texture stuff needs to happen after it
    InitWindow(screenWidth, screenHeight, "WaterEmblemProto");
    Terrain Plains = { .id = 0, .color = GREEN, .sprite = LoadTextureFromImage(plains_sprite), .name = "Plains"};
    Terrain Mountains = { .id = 1, .color = LIGHTGRAY, .sprite = LoadTextureFromImage(mountains_sprite), .name = "Mountains" };
    Terrain Sea = { .id = 2, .color = DARKBLUE, .sprite = LoadTextureFromImage(sea_sprite), .name = "Sea" };
    Terrain Arctic = { .id = 3, .color = WHITE, .sprite = LoadTextureFromImage(arctic_sprite), .name = "Arctic" };
    Terrain Forest = { .id = 4, .color = DARKGREEN, .sprite = LoadTextureFromImage(forest_sprite), .name = "Forest" };
    
    // Configure each biome type
    BiomeConfig biome_configs[] = {
        { .terrain = Mountains, .max_cores = 3, .max_range = 3 },
        { .terrain = Arctic, .max_cores = 3, .max_range = 3 },
        { .terrain = Forest, .max_cores = 3, .max_range = 3 },
        { .terrain = Sea,    .max_cores = 2, .max_range = 5 }
    };
    
    
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
    Player factions[] = {
        { .has_turn = true, .prim_color = PURPLE, .sec_color = DARKGRAY, .name = "Darkus" },
        { .has_turn = false, .prim_color = GREEN, .sec_color = WHITE, .name = "Ventus" },
        { .has_turn = false, .prim_color = BROWN, .sec_color = BLACK, .name = "Gaia" }
    };
    
    Actor * dark_troops = malloc(sizeof(Actor)*DARK_TROOP_NUM);
    Actor * vent_troops = malloc(sizeof(Actor)*VENT_TROOP_NUM);

    Texture2D d_militia_text = LoadTextureFromImage(d_militia_sprite);
    Texture2D v_militia_text = LoadTextureFromImage(v_militia_sprite);

    for (int i = 0; i < DARK_TROOP_NUM; i++) {
        actor_init(dark_troops + i, factions + 0, d_militia_text);
        Point * spawn = get_random_spawn_cell(mapArr);
        spawn->occupant = dark_troops + i;
    }

    for (int i = 0; i < VENT_TROOP_NUM; i++) {
        actor_init(vent_troops + i, factions + 0, v_militia_text);
        Point * spawn = get_random_spawn_cell(mapArr);
        spawn->occupant = vent_troops + i;
    }
    // Init current player state

    Player * curr_player = factions + 0;
    Point * focused_cell = NULL;
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // The XY coords are in the top left corner of the square
        // LMB
        Point * selected_cell = mouseToCell(gridPosition, mapArr);
        if (IsMouseButtonPressed(0)) {
            // tells us which cell we have selected in the mapArr
            // had to do pointer stuff to point to the permanent object
            cell_selection(mapArr, selected_cell, &focused_cell);
        }
        // RMB
        else if (IsMouseButtonPressed(1)) {
            if ((focused_cell->occupant != NULL) && selected_cell->in_range){
                // removed due to flushing function
                // range_calc(mapArr, last_player_position, player.movement, false);
                selected_cell->occupant = focused_cell->occupant;
                focused_cell->occupant = NULL;
                
                focused_cell = NULL;
            }
            cell_flag_flush(mapArr); // has to be after so player can move
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
            
            // first we draw terrain color for failsafe, then texture
            DrawRectangle(cell_x_pos, cell_y_pos,
                GRID_CELL_SIZE, GRID_CELL_SIZE, curr_cell.terrain.color);
            
            DrawTexture(curr_cell.terrain.sprite, cell_x_pos, cell_y_pos, WHITE);
                
                
            if (curr_cell.occupant != NULL) {
                Actor* occupant = curr_cell.occupant;
                DrawTexture(occupant->sprite, cell_x_pos, cell_y_pos, WHITE);
            }

            
            DrawRectangleLines(cell_x_pos, cell_y_pos,
                GRID_CELL_SIZE, GRID_CELL_SIZE, GRAY);
                
            if (curr_cell.occupant != NULL) {
                DrawRectangleLines(cell_x_pos, cell_y_pos,
                    GRID_CELL_SIZE, GRID_CELL_SIZE, curr_cell.occupant->owner->prim_color);
                }
            if (curr_cell.in_range == true) {
                DrawRectangleLines(cell_x_pos, cell_y_pos,
                    GRID_CELL_SIZE, GRID_CELL_SIZE, BLUE);
            }

            if (curr_cell.in_attack_range == true) {
                DrawRectangleLines(cell_x_pos, cell_y_pos,
                    GRID_CELL_SIZE, GRID_CELL_SIZE, RED);
            }
            }
        focused_cell_info(focused_cell, gridPosition);
                    
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
void cell_selection(Point * cell_arr, Point * cell, Point ** focused_cell) {
    // flushes the map of range indicator
    cell_flag_flush(cell_arr);
    *focused_cell = cell;
    if (cell->occupant != NULL) {
        range_calc(cell_arr, cell, cell->occupant->movement, true);
        attack_range_calc(cell_arr, cell, cell->occupant->range, true);
    }
    return;
}

void range_calc(Point * cell_arr, Point * start_cell, int range, bool selection) {
    // quick fix, also removes targeting and affects flyers
    // comparison problem, will have to refactor this
    if (start_cell->terrain.id == 2) {
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

// calcs attack range
void attack_range_calc(Point * cell_arr, Point * start_cell, int range, bool selection) {
    start_cell->in_attack_range = selection;
    if (range == 0) {
        return;
    }
    int x = start_cell->x;
    int y = start_cell->y;
    int x_limit = MAX_GRID_CELLS_X - 1;
    int y_limit = MAX_GRID_CELLS_Y - 1;
    if (y != 0) {
        Point * cell_up = cell_arr + x + MAX_GRID_CELLS_X*(y - 1);
        attack_range_calc(cell_arr, cell_up, range - 1, selection);

    }
    if (y < y_limit) {

        Point * cell_down = cell_arr + x + MAX_GRID_CELLS_X*(y + 1);
        attack_range_calc(cell_arr, cell_down, range - 1, selection);
    }
    if (x != 0) {
        Point * cell_left = cell_arr + x - 1 + MAX_GRID_CELLS_X*(y);
        attack_range_calc(cell_arr, cell_left, range - 1, selection);
    }
    if (x < x_limit) {
        Point * cell_right = cell_arr + x + 1 + MAX_GRID_CELLS_X*(y);
        attack_range_calc(cell_arr, cell_right, range - 1, selection);
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

void actor_init( Actor * actor, Player * owner, Texture2D sprite) {
    actor->sprite = sprite;
    strcpy(actor->name, "Azao");
    actor->owner = owner;
    actor->can_move = true;
    actor->can_act = true;

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

void focused_cell_info(Point * selected_cell, Vector2 gridPosition) {
    if (selected_cell == NULL) { return; }
    if (selected_cell->occupant != NULL) {
        Actor * occupant = selected_cell->occupant;
        DrawText(TextFormat(
            "NAME: %s\nFAC: %s\nLVL: %d\nEXP NEEDED: %d\nHP: %d/%d\nMOV: %d\nATK: %d\nDEF: %d\n",
            occupant->name, occupant->owner, occupant->level, occupant->next_level_xp,
            occupant->curr_health, occupant->max_health,
            occupant->movement, occupant->attack, occupant->defense),
            MAX_GRID_CELLS_X*GRID_CELL_SIZE + gridPosition.x + 20, gridPosition.y, 20, BLACK);
    }
    else {
        DrawText(TextFormat(
            "TRN: %d\n",
            selected_cell->terrain.name),
            MAX_GRID_CELLS_X*GRID_CELL_SIZE + gridPosition.x + 20, gridPosition.y, 20, BLACK);
    }

    // so we know which selected
    DrawRectangleLines(selected_cell->x*GRID_CELL_SIZE + gridPosition.x,
        selected_cell->y*GRID_CELL_SIZE + gridPosition.y,
        GRID_CELL_SIZE, GRID_CELL_SIZE, YELLOW);
    return;
}

void cell_flag_flush(Point * cell_arr) {
    for (int yCoor = 0; yCoor < MAX_GRID_CELLS_Y; yCoor++) {
        for (int xCoor = 0; xCoor < MAX_GRID_CELLS_X; xCoor++) {
            cell_arr[xCoor + yCoor*MAX_GRID_CELLS_X].in_range = false;
            cell_arr[xCoor + yCoor*MAX_GRID_CELLS_X].in_attack_range = false;
        }
    }
}

Point * get_random_spawn_cell(Point * cell_arr) {
    Point * cell = get_random_cell(cell_arr);
    while (cell->terrain.id == 2 || cell->occupant != NULL) {
        cell = get_random_cell(cell_arr);
    }
    return cell;
}