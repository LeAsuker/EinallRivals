#include "ui/modal.h"
#include "game/actor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <raylib.h>

/**
 * @brief Allocate and initialize a Modal instance with defaults.
 * @return Pointer to newly allocated Modal, or NULL on allocation failure.
 */
Modal *modal_create(void) {
    Modal *modal = (Modal *)calloc(1, sizeof(Modal));
    modal->type = MODAL_TYPE_NONE;
    modal->active = false;
    modal->buttons = NULL;
    modal->button_count = 0;
    modal->result = MODAL_RESULT_NONE;
    modal->overlay_color = (Color){0, 0, 0, 180};  // Semi-transparent black
    return modal;
}

/**
 * @brief Free a Modal and any owned resources such as dynamic buttons.
 * @param modal Modal to free (NULL-safe).
 */
void modal_free(Modal *modal) {
    if (!modal) return;
    if (modal->buttons) {
        free(modal->buttons);
    }
    free(modal);
}

/**
 * @brief Clear modal state, free buttons, and deactivate the modal.
 * @param modal Modal to clear (NULL-safe).
 */
void modal_clear(Modal *modal) {
    if (!modal) return;
    modal->active = false;
    modal->type = MODAL_TYPE_NONE;
    modal->result = MODAL_RESULT_NONE;
    if (modal->buttons) {
        free(modal->buttons);
        modal->buttons = NULL;
    }
    modal->button_count = 0;
    modal->data = NULL;
}

/**
 * @brief Configure a simple pause/ESC modal with Resume/Exit options.
 * @param modal Modal to configure (must be valid).
 * @param screen_width Screen width for centering computations.
 * @param screen_height Screen height for centering computations.
 */
void modal_setup_esc_menu(Modal *modal, int screen_width, int screen_height) {
    NULL_CHECK_VOID(modal);
    modal_clear(modal);
    
    modal->type = MODAL_TYPE_ESC_MENU;
    modal->active = true;
    
    // Center the modal
    int modal_width = 400;
    int modal_height = 300;
    modal->bounds = (Rectangle){
        (screen_width - modal_width) / 2.0f,
        (screen_height - modal_height) / 2.0f,
        modal_width,
        modal_height
    };
    modal->bg_color = RAYWHITE;
    
    strcpy(modal->title, "PAUSED");
    strcpy(modal->description, "");
    
    // Create buttons
    modal->button_count = 3;
    modal->buttons = (Button *)calloc(modal->button_count, sizeof(Button));
    
    int button_width = 300;
    int button_height = 50;
    int button_x = modal->bounds.x + (modal->bounds.width - button_width) / 2;
    int button_y = modal->bounds.y + 80;
    int button_spacing = 70;
    
    // Resume button
    button_init(&modal->buttons[0], button_x, button_y, button_width, button_height);
    button_set_label(&modal->buttons[0], "Resume", 24, BLACK);
    button_set_colors(&modal->buttons[0], LIGHTGRAY, DARKGRAY);
    
    // Exit to Menu button
    button_init(&modal->buttons[1], button_x, button_y + button_spacing, button_width, button_height);
    button_set_label(&modal->buttons[1], "Exit to Menu", 24, BLACK);
    button_set_colors(&modal->buttons[1], LIGHTGRAY, DARKGRAY);
    
    // Exit to Desktop button
    button_init(&modal->buttons[2], button_x, button_y + button_spacing * 2, button_width, button_height);
    button_set_label(&modal->buttons[2], "Exit to Desktop", 24, BLACK);
    button_set_colors(&modal->buttons[2], LIGHTGRAY, DARKGRAY);
}

/**
 * @brief Setup a level-up modal populated with class choice buttons.
 * @param modal Modal to configure (must be valid).
 * @param screen_width Screen width for centering layout.
 * @param screen_height Screen height for centering layout.
 * @param character Character being leveled up (displayed in the title).
 * @param class_options Array of option strings to use as button labels.
 * @param class_count Number of class options.
 */
void modal_setup_level_up(Modal *modal, int screen_width, int screen_height,
                          Character *character, const char **class_options, int class_count) {
    NULL_CHECK_VOID(modal);
    NULL_CHECK_VOID(character);
    NULL_CHECK_VOID(class_options);
    modal_clear(modal);
    
    modal->type = MODAL_TYPE_LEVEL_UP;
    modal->active = true;
    modal->data = character;  // Store character reference
    
    // Larger modal for class choices
    int modal_width = 600;
    int modal_height = 400;
    modal->bounds = (Rectangle){
        (screen_width - modal_width) / 2.0f,
        (screen_height - modal_height) / 2.0f,
        modal_width,
        modal_height
    };
    modal->bg_color = RAYWHITE;
    
    snprintf(modal->title, sizeof(modal->title), "%s LEVEL UP!", character->name);
    snprintf(modal->description, sizeof(modal->description), 
             "Choose your promotion class:");
    
    // Create class choice buttons
    modal->button_count = class_count;
    modal->buttons = (Button *)calloc(modal->button_count, sizeof(Button));
    
    int button_width = 250;
    int button_height = 60;
    int button_spacing = 80;
    int start_y = modal->bounds.y + 120;
    
    for (int i = 0; i < class_count; i++) {
        int button_x = modal->bounds.x + (modal->bounds.width - button_width) / 2;
        int button_y = start_y + i * button_spacing;
        
        button_init(&modal->buttons[i], button_x, button_y, button_width, button_height);
        button_set_label(&modal->buttons[i], class_options[i], 22, BLACK);
        button_set_colors(&modal->buttons[i], (Color){200, 220, 255, 255}, DARKBLUE);
        button_set_border_thickness(&modal->buttons[i], 3);
    }
}

/**
 * @brief Update modal input and button state, returning any result produced.
 * @param modal Modal to update.
 * @return A ModalResult value indicating a selection or MODAL_RESULT_NONE.
 */
ModalResult modal_update(Modal *modal) {
    if (!modal || !modal->active) {
        return MODAL_RESULT_NONE;
    }
    
    modal->result = MODAL_RESULT_NONE;
    
    bool mouse_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool mouse_pressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // Update and check button clicks
    for (int i = 0; i < modal->button_count; i++) {
        button_update(&modal->buttons[i], mouse_down);
        
        // Check if button was just clicked (pressed and mouse released)
        if (modal->buttons[i].pressed && mouse_pressed) {
            switch (modal->type) {
                case MODAL_TYPE_ESC_MENU:
                    if (i == 0) modal->result = MODAL_RESULT_CANCEL;
                    else if (i == 1) modal->result = MODAL_RESULT_EXIT_TO_MENU;
                    else if (i == 2) modal->result = MODAL_RESULT_EXIT_TO_DESKTOP;
                    break;
                    
                case MODAL_TYPE_LEVEL_UP:
                    modal->result = MODAL_RESULT_CLASS_CHOICE_0 + i;
                    break;
                    
                default:
                    break;
            }
            
            if (modal->result != MODAL_RESULT_NONE) {
                modal->active = false;
                return modal->result;
            }
        }
    }
    
    // ESC key closes ESC menu
    if (modal->type == MODAL_TYPE_ESC_MENU && IsKeyPressed(KEY_ESCAPE)) {
        modal->result = MODAL_RESULT_CANCEL;
        modal->active = false;
    }
    
    return modal->result;
}

/**
 * @brief Render an active modal dialog including overlay, background, text, and buttons.
 * @param modal Modal to render (must be non-NULL and active to draw anything).
 */
void modal_render(const Modal *modal) {
    if (!modal || !modal->active) return;
    
    // Draw dimming overlay over entire screen
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), modal->overlay_color);
    
    // Draw modal background
    DrawRectangleRec(modal->bounds, modal->bg_color);
    DrawRectangleLinesEx(modal->bounds, 4, BLACK);
    
    // Draw title
    int title_size = 32;
    int title_width = MeasureText(modal->title, title_size);
    DrawText(modal->title, 
             modal->bounds.x + (modal->bounds.width - title_width) / 2,
             modal->bounds.y + 20,
             title_size, BLACK);
    
    // Draw description
    if (modal->description[0] != '\0') {
        int desc_size = 20;
        int desc_width = MeasureText(modal->description, desc_size);
        DrawText(modal->description,
                 modal->bounds.x + (modal->bounds.width - desc_width) / 2,
                 modal->bounds.y + 60,
                 desc_size, DARKGRAY);
    }
    
    // Draw buttons
    for (int i = 0; i < modal->button_count; i++) {
        button_draw(&modal->buttons[i]);
    }
}
