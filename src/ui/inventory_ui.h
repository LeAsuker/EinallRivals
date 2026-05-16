#ifndef INVENTORY_UI_H_
#define INVENTORY_UI_H_

#include "types.h"
#include "ui/button.h"
#include <raylib.h>
#include <stdbool.h>

typedef struct InventoryUI {
  bool active;
  Character *character;
  Button equip_buttons[MAX_EQUIP_SLOTS];
  Button inv_buttons[INVENTORY_SIZE];
  Button destroy_button;
  Button close_button;
  int selected_equip;
  int selected_inv;
  Rectangle bounds;
  Color bg_color;
  Color overlay_color;
  char message[128];
} InventoryUI;

InventoryUI *inventory_ui_create(void);
void inventory_ui_free(InventoryUI *ui);
void inventory_ui_open(InventoryUI *ui, Character *character, int screen_w,
                       int screen_h);
void inventory_ui_close(InventoryUI *ui);
bool inventory_ui_update(InventoryUI *ui);
void inventory_ui_render(InventoryUI *ui);

#endif
