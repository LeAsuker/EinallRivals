#include "ui/button.h"
#include "raylib.h"
#include "types.h"
#include <string.h>

bool button_is_mouse_over(const Button *b) {
    NULL_CHECK_RET(b, false);
    int mx = GetMouseX();
    int my = GetMouseY();
    return (mx >= b->x && mx < b->x + b->width &&
            my >= b->y && my < b->y + b->height);
}

void button_update(Button *b, bool mouse_down) {
    NULL_CHECK_VOID(b);
    // Pressed when mouse is down while over the button
    if (button_is_mouse_over(b) && mouse_down) {
        b->pressed = true;
    } else {
        b->pressed = false;
    }
}

void button_draw(const Button *b) {
    NULL_CHECK_VOID(b);
    // Draw background
    DrawRectangle(b->x, b->y, b->width, b->height, b->bg_color);

    // Draw border using thickness
    for (int i = 0; i < b->border_thickness; i++) {
        DrawRectangleLines(b->x - i, b->y - i, b->width + i * 2, b->height + i * 2, b->border_color);
    }

    // Draw label centered if present
    if (b->label[0] != '\0') {
        int text_w = MeasureText(b->label, b->text_size);
        int tx = b->x + (b->width - text_w) / 2;
        int ty = b->y + (b->height - b->text_size) / 2;
        DrawText(b->label, tx, ty, b->text_size, b->text_color);
    }
}

// No-op color setter here since header exposes inline setter; kept for compatibility
// (defined inline in header)
