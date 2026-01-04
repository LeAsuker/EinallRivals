#ifndef UI_BUTTON_H_
#define UI_BUTTON_H_

#include <stdbool.h>

// Simple UI button primitive used for interactive elements outside the map.
// Coordinates are in pixels (screen space).
typedef struct Button {
    int x;
    int y;
    int width;
    int height;
    bool pressed;    // True while mouse is held down over the button
} Button;

// Initialize a button's rectangle and pressed state.
static inline void button_init(Button *b, int x, int y, int width, int height) {
    b->x = x; b->y = y; b->width = width; b->height = height; b->pressed = false;
}

// Set the button rectangle (useful if UI layout changes each frame)
static inline void button_set_rect(Button *b, int x, int y, int width, int height) {
    b->x = x; b->y = y; b->width = width; b->height = height;
}

// Returns true if the current mouse position is inside the button rect.
bool button_is_mouse_over(const Button *b);

// Update the pressed state based on whether the mouse is currently down and
// whether the cursor is over the button. Call every frame before rendering.
void button_update(Button *b, bool mouse_down);

#endif // UI_BUTTON_H_
