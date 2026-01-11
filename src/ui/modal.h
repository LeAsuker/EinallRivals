#ifndef MODAL_H
#define MODAL_H

#include "types.h"
#include "ui/button.h"
#include <raylib.h>
#include <stdbool.h>

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

/**
 * @brief Modal dialog container used for various in-game overlays (pause menu,
 * level up, etc.).
 */
typedef struct Modal {
  ModalType type; /**< Type of modal content being displayed. */
  bool active; /**< Whether modal is active and should be rendered/processed. */

  /* Generic data pointer for modal-specific context */
  void *data;

  /* Layout */
  Rectangle bounds;    /**< Background rectangle for the modal dialog. */
  Color bg_color;      /**< Background color for the modal. */
  Color overlay_color; /**< Dimming overlay color drawn behind the modal. */

  /* Buttons (dynamic array) */
  Button *buttons;  /**< Dynamically allocated buttons for the modal. */
  int button_count; /**< Number of buttons in the array. */

  /* Title and text */
  char title[128];       /**< Modal title string. */
  char description[256]; /**< Descriptive text or instructions. */

  /* Result */
  ModalResult result; /**< Last result produced by modal_update.
                         Set when a button is activated or modal is closed. */
} Modal;

/**
 * @brief Allocate and initialize an empty Modal (caller should free with
 * modal_free).
 * @return A newly allocated Modal pointer, or NULL on allocation failure.
 */
Modal *modal_create(void);

/**
 * @brief Free a Modal and any associated dynamic resources (buttons).
 * @param modal Modal to free (NULL-safe).
 */
void modal_free(Modal *modal);

/**
 * @brief Clear a modal's state, deactivate it and free its buttons.
 * @param modal Modal to clear (NULL-safe).
 */
void modal_clear(Modal *modal);

/**
 * @brief Setup a simple escape menu modal (resume/exit options).
 * @param modal Modal to populate.
 * @param screen_width Screen width to compute centered bounds.
 * @param screen_height Screen height to compute centered bounds.
 */
void modal_setup_esc_menu(Modal *modal, int screen_width, int screen_height);

/**
 * @brief Setup a level-up modal presenting class choice buttons.
 * @param modal Modal to populate.
 * @param screen_width Screen width for layout.
 * @param screen_height Screen height for layout.
 * @param character Character being leveled up (displayed/affected).
 * @param class_options Array of option strings to show as buttons.
 * @param class_count Number of class options.
 */
void modal_setup_level_up(Modal *modal, int screen_width, int screen_height,
                          Character *character, const char **class_options,
                          int class_count);

/**
 * @brief Update the modal state, processing input and returning any result.
 * @param modal Modal to update.
 * @return ModalResult indicating button activation or MODAL_RESULT_NONE if
 * none.
 */
ModalResult modal_update(Modal *modal);

/**
 * @brief Render the modal (overlay, background, title, description, buttons).
 * @param modal Modal to render (only active modals are drawn).
 */
void modal_render(const Modal *modal);

#endif // MODAL_H
