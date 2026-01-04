#include "ui/button.h"
#include "raylib.h"

bool button_is_mouse_over(const Button *b) {
    int mx = GetMouseX();
    int my = GetMouseY();
    return (mx >= b->x && mx < b->x + b->width &&
            my >= b->y && my < b->y + b->height);
}

void button_update(Button *b, bool mouse_down) {
    // Pressed when mouse is down while over the button
    if (button_is_mouse_over(b) && mouse_down) {
        b->pressed = true;
    } else {
        b->pressed = false;
    }
}
