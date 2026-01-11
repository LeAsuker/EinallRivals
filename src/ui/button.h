#ifndef UI_BUTTON_H_
#define UI_BUTTON_H_

#include <stdbool.h>
#include "raylib.h"
#include <stddef.h>

/**
 * @brief Simple UI button primitive used for interactive elements outside the map.
 * Coordinates are in pixels (screen space).
 */
typedef struct Button {
    int x;                    /**< Top-left X coordinate (pixels) */
    int y;                    /**< Top-left Y coordinate (pixels) */
    int width;                /**< Width in pixels */
    int height;               /**< Height in pixels */
    bool pressed;             /**< True while mouse is held down over the button */

    /* Visual properties */
    Color bg_color;           /**< Background fill color */
    Color border_color;       /**< Border outline color */
    int border_thickness;     /**< Border thickness in pixels */
    
    /* Optional text label (empty string means no text). Rendered centered. */
    char label[64];           /**< Label buffer (null-terminated) */
    Color text_color;         /**< Color used for the label text */
    int text_size;            /**< Font size of the label text */
} Button;

/**
 * @brief Initialize a button's rectangle and pressed state with sensible defaults.
 * @param b Button to initialize.
 * @param x Top-left X coordinate in pixels.
 * @param y Top-left Y coordinate in pixels.
 * @param width Width in pixels.
 * @param height Height in pixels.
 */
static inline void button_init(Button *b, int x, int y, int width, int height) {
    b->x = x; b->y = y; b->width = width; b->height = height; b->pressed = false;
    b->bg_color = (Color){200,200,200,255};
    b->border_color = BLACK;
    b->border_thickness = 1;
    b->label[0] = '\0';
    b->text_color = BLACK;
    b->text_size = 20;
}

/**
 * @brief Set the button rectangle (useful if UI layout changes each frame).
 * @param b Button to modify.
 * @param x New top-left X coordinate.
 * @param y New top-left Y coordinate.
 * @param width New width in pixels.
 * @param height New height in pixels.
 */
static inline void button_set_rect(Button *b, int x, int y, int width, int height) {
    b->x = x; b->y = y; b->width = width; b->height = height;
}

/**
 * @brief Set the button colors (background and border).
 * @param b Button to modify.
 * @param bg Background color.
 * @param border Border color.
 */
static inline void button_set_colors(Button *b, Color bg, Color border) {
    b->bg_color = bg;
    b->border_color = border;
}

/**
 * @brief Set border thickness in pixels.
 * @param b Button to modify.
 * @param thickness Thickness in pixels.
 */
static inline void button_set_border_thickness(Button *b, int thickness) {
    b->border_thickness = thickness;
}

/**
 * @brief Set the button label (copies into the internal buffer).
 *        Pass NULL or empty string to clear the label. `text_size` is font size in pixels.
 * @param b Button to modify.
 * @param text Label string (may be NULL).
 * @param text_size Font size in pixels.
 * @param text_color Color for the label text.
 */
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

/**
 * @brief Returns true if the current mouse position is inside the button rect.
 * @param b Button to test.
 * @return true if mouse is over the button.
 */
bool button_is_mouse_over(const Button *b);

/**
 * @brief Update the pressed state based on whether the mouse is currently down and
 *        whether the cursor is over the button. Call every frame before rendering.
 * @param b Button to update.
 * @param mouse_down true if the mouse button is currently down.
 */
void button_update(Button *b, bool mouse_down);

/**
 * @brief Draw the button (background + border). Does not draw any text.
 * @param b Button to draw.
 */
void button_draw(const Button *b);

#endif // UI_BUTTON_H_
