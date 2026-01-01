#include "ui/menu.h"
#include "raylib.h"

#define MENU_BUTTON_WIDTH 200
#define MENU_BUTTON_HEIGHT 50
#define MENU_BUTTON_SPACING 20

void menu_init(MenuState *state) {
    state->selected_option = MENU_NONE;
    state->hovered_option = MENU_NONE;
    state->is_active = true;
}

void menu_update(MenuState *state) {
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    
    // Calculate button positions
    int center_x = screen_width / 2;
    int start_y = screen_height / 2 + 50;
    
    // Start button bounds
    int start_button_x = center_x - MENU_BUTTON_WIDTH / 2;
    int start_button_y = start_y;
    
    // Quit button bounds
    int quit_button_x = center_x - MENU_BUTTON_WIDTH / 2;
    int quit_button_y = start_y + MENU_BUTTON_HEIGHT + MENU_BUTTON_SPACING;
    
    // Check hover state
    state->hovered_option = MENU_NONE;
    
    if (mouse_x >= start_button_x && mouse_x <= start_button_x + MENU_BUTTON_WIDTH &&
        mouse_y >= start_button_y && mouse_y <= start_button_y + MENU_BUTTON_HEIGHT) {
        state->hovered_option = MENU_START;
    }
    else if (mouse_x >= quit_button_x && mouse_x <= quit_button_x + MENU_BUTTON_WIDTH &&
             mouse_y >= quit_button_y && mouse_y <= quit_button_y + MENU_BUTTON_HEIGHT) {
        state->hovered_option = MENU_QUIT;
    }
    
    // Check for click
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (state->hovered_option != MENU_NONE) {
            state->selected_option = state->hovered_option;
            state->is_active = false;
        }
    }
}

void menu_render(MenuState *state, int screen_width, int screen_height) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Draw game title
    const char *title = "Einall Rivals";
    int title_font_size = 80;
    int title_width = MeasureText(title, title_font_size);
    DrawText(title, screen_width / 2 - title_width / 2, screen_height / 3, 
             title_font_size, DARKBLUE);
    
    // Draw creator name
    const char *creator = "Created by: Asuker";
    int creator_font_size = 20;
    int creator_width = MeasureText(creator, creator_font_size);
    DrawText(creator, screen_width / 2 - creator_width / 2, 
             screen_height / 3 + 100, creator_font_size, DARKGRAY);
    
    // Calculate button positions
    int center_x = screen_width / 2;
    int start_y = screen_height / 2 + 50;
    
    int start_button_x = center_x - MENU_BUTTON_WIDTH / 2;
    int start_button_y = start_y;
    
    int quit_button_x = center_x - MENU_BUTTON_WIDTH / 2;
    int quit_button_y = start_y + MENU_BUTTON_HEIGHT + MENU_BUTTON_SPACING;
    
    // Draw Start button
    Color start_color = (state->hovered_option == MENU_START) ? GREEN : DARKGREEN;
    DrawRectangle(start_button_x, start_button_y, 
                  MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, start_color);
    DrawRectangleLines(start_button_x, start_button_y, 
                       MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, BLACK);
    
    const char *start_text = "START GAME";
    int start_text_width = MeasureText(start_text, 20);
    DrawText(start_text, 
             start_button_x + MENU_BUTTON_WIDTH / 2 - start_text_width / 2,
             start_button_y + MENU_BUTTON_HEIGHT / 2 - 10,
             20, WHITE);
    
    // Draw Quit button
    Color quit_color = (state->hovered_option == MENU_QUIT) ? RED : DARKPURPLE;
    DrawRectangle(quit_button_x, quit_button_y, 
                  MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, quit_color);
    DrawRectangleLines(quit_button_x, quit_button_y, 
                       MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, BLACK);
    
    const char *quit_text = "QUIT";
    int quit_text_width = MeasureText(quit_text, 20);
    DrawText(quit_text, 
             quit_button_x + MENU_BUTTON_WIDTH / 2 - quit_text_width / 2,
             quit_button_y + MENU_BUTTON_HEIGHT / 2 - 10,
             20, WHITE);
    
    EndDrawing();
}

MenuOption menu_get_selected(MenuState *state) {
    return state->selected_option;
}
