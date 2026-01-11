#ifndef MENU_H_
#define MENU_H_

#include "raylib.h"
#include <stdbool.h>

typedef enum { MENU_NONE, MENU_START, MENU_QUIT } MenuOption;

typedef struct {
  MenuOption selected_option; /**< Option that was chosen (after confirm) */
  MenuOption hovered_option;  /**< Option currently hovered by mouse */
  bool is_active; /**< Whether the menu is currently active and accepting input
                   */
} MenuState;

/**
 * @brief Initialize a MenuState to default values.
 * @param state Menu state to initialize (must be valid).
 */
void menu_init(MenuState *state);

/**
 * @brief Update menu input and hovered/selected states; call each frame while
 * menu active.
 * @param state Menu state to update.
 */
void menu_update(MenuState *state);

/**
 * @brief Render the main menu UI.
 * @param state MenuState to render.
 * @param screen_width Width of the screen (used for centering calculations).
 * @param screen_height Height of the screen.
 */
void menu_render(MenuState *state, int screen_width, int screen_height);

/**
 * @brief Return the selected menu option.
 * @param state MenuState to query.
 * @return Selected MenuOption (MENU_NONE if none selected).
 */
MenuOption menu_get_selected(MenuState *state);

#endif
