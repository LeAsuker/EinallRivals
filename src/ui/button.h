#ifndef UI_BUTTON_H_
#define UI_BUTTON_H_

#include <stdbool.h>
#include "raylib.h"
#include <stddef.h>

// Simple UI button primitive used for interactive elements outside the map.
// Coordinates are in pixels (screen space).
typedef struct Button {
    int x;
    int y;
    int width;
    int height;
    bool pressed;    // True while mouse is held down over the button

    // Visual properties
    Color bg_color;
    Color border_color;
    int border_thickness;
    
    // Optional text label (empty string means no text). Rendered centered.
    char label[64];
    Color text_color;
    int text_size;
} Button;

// Initialize a button's rectangle and pressed state with sensible defaults.
static inline void button_init(Button *b, int x, int y, int width, int height) {
    b->x = x; b->y = y; b->width = width; b->height = height; b->pressed = false;
    b->bg_color = (Color){200,200,200,255};
    b->border_color = BLACK;
    b->border_thickness = 1;
    b->label[0] = '\0';
    b->text_color = BLACK;
    b->text_size = 20;
}

// Set the button rectangle (useful if UI layout changes each frame)
static inline void button_set_rect(Button *b, int x, int y, int width, int height) {
    b->x = x; b->y = y; b->width = width; b->height = height;
}

// Set the button colors (background and border)
static inline void button_set_colors(Button *b, Color bg, Color border) {
    b->bg_color = bg;
    b->border_color = border;
}

// Set border thickness in pixels
static inline void button_set_border_thickness(Button *b, int thickness) {
    b->border_thickness = thickness;
}

// Set the button label (copies into the internal buffer). Pass NULL or
// empty string to clear the label. `text_size` is font size in pixels.
static inline void button_set_label(Button *b, const char *text, int text_size, Color text_color) {
    if (text == NULL) {
        b->label[0] = '\0';
    } else {
        int max = (int)sizeof(b->label) - 1;
        int i;
        for (i = 0; i < max && text[i] != '\0'; i++) b->label[i] = text[i];
        b->label[i] = '\0';
    }
    b->text_size = text_size;
    b->text_color = text_color;
}

// Returns true if the current mouse position is inside the button rect.
bool button_is_mouse_over(const Button *b);

// Update the pressed state based on whether the mouse is currently down and
// whether the cursor is over the button. Call every frame before rendering.
void button_update(Button *b, bool mouse_down);

// Draw the button (background + border). Does not draw any text.
void button_draw(const Button *b);

#endif // UI_BUTTON_H_
