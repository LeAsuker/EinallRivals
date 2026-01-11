#ifndef MODAL_H
#define MODAL_H

#include "types.h"
#include "ui/button.h"
#include <stdbool.h>
#include <raylib.h>

typedef enum {
    MODAL_TYPE_NONE,
    MODAL_TYPE_ESC_MENU,
    MODAL_TYPE_LEVEL_UP,
    // Add more types as needed
} ModalType;

typedef enum {
    MODAL_RESULT_NONE,
    MODAL_RESULT_EXIT_TO_MENU,
    MODAL_RESULT_EXIT_TO_DESKTOP,
    MODAL_RESULT_CANCEL,
    MODAL_RESULT_CLASS_CHOICE_0,
    MODAL_RESULT_CLASS_CHOICE_1,
    MODAL_RESULT_CLASS_CHOICE_MAX = MODAL_RESULT_CLASS_CHOICE_1 + 10,
} ModalResult;

typedef struct Modal {
    ModalType type;
    bool active;
    
    // Generic data pointer for modal-specific context
    void *data;
    
    // Layout
    Rectangle bounds;
    Color bg_color;
    Color overlay_color;  // Dimming overlay
    
    // Buttons (dynamic array)
    Button *buttons;
    int button_count;
    
    // Title and text
    char title[128];
    char description[256];
    
    // Result
    ModalResult result;
} Modal;

// Lifecycle
Modal *modal_create(void);
void modal_free(Modal *modal);
void modal_clear(Modal *modal);

// Setup functions
void modal_setup_esc_menu(Modal *modal, int screen_width, int screen_height);
void modal_setup_level_up(Modal *modal, int screen_width, int screen_height, 
                          Character *character, const char **class_options, int class_count);

// Update and input
ModalResult modal_update(Modal *modal);

// Rendering
void modal_render(const Modal *modal);

#endif // MODAL_H
