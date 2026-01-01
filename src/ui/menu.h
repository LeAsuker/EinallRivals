#ifndef MENU_H_
#define MENU_H_

#include "raylib.h"
#include <stdbool.h>

typedef enum {
    MENU_NONE,
    MENU_START,
    MENU_QUIT
} MenuOption;

typedef struct {
    MenuOption selected_option;
    MenuOption hovered_option;
    bool is_active;
} MenuState;

void menu_init(MenuState *state);
void menu_update(MenuState *state);
void menu_render(MenuState *state, int screen_width, int screen_height);
MenuOption menu_get_selected(MenuState *state);

#endif
