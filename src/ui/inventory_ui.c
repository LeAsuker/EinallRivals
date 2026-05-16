#include "ui/inventory_ui.h"
#include "game/item.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INV_W 700
#define INV_H 500
#define SLOT_W 280
#define SLOT_H 40
#define COL1_X 40
#define COL2_X 380
#define START_Y 80
#define GAP_Y 55

InventoryUI *inventory_ui_create(void) {
  InventoryUI *ui = calloc(1, sizeof(InventoryUI));
  if (!ui)
    return NULL;
  ui->active = false;
  ui->overlay_color = (Color){0, 0, 0, 180};
  ui->bg_color = RAYWHITE;
  ui->selected_equip = -1;
  ui->selected_inv = -1;
  return ui;
}

void inventory_ui_free(InventoryUI *ui) { free(ui); }

void inventory_ui_open(InventoryUI *ui, Character *character, int screen_w,
                       int screen_h) {
  if (!ui)
    return;
  ui->active = true;
  ui->character = character;
  ui->selected_equip = -1;
  ui->selected_inv = -1;
  ui->message[0] = '\0';
  ui->bounds =
      (Rectangle){(screen_w - INV_W) / 2.0f, (screen_h - INV_H) / 2.0f, INV_W,
                  INV_H};

  int y = ui->bounds.y + START_Y;
  for (int i = 0; i < MAX_EQUIP_SLOTS; i++) {
    button_init(&ui->equip_buttons[i], ui->bounds.x + COL1_X,
                y + i * GAP_Y, SLOT_W, SLOT_H);
    button_set_colors(&ui->equip_buttons[i], LIGHTGRAY, DARKGRAY);
  }
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    button_init(&ui->inv_buttons[i], ui->bounds.x + COL2_X, y + i * GAP_Y,
                SLOT_W, SLOT_H);
    button_set_colors(&ui->inv_buttons[i], LIGHTGRAY, DARKGRAY);
  }

  int btn_y = ui->bounds.y + INV_H - 70;
  button_init(&ui->destroy_button, ui->bounds.x + COL1_X, btn_y, 120, 40);
  button_set_label(&ui->destroy_button, "Destroy", 18, BLACK);
  button_set_colors(&ui->destroy_button, (Color){255, 150, 150, 255}, DARKGRAY);

  button_init(&ui->close_button, ui->bounds.x + COL2_X + SLOT_W - 120, btn_y,
              120, 40);
  button_set_label(&ui->close_button, "Close", 18, BLACK);
  button_set_colors(&ui->close_button, (Color){150, 255, 150, 255}, DARKGRAY);
}

void inventory_ui_close(InventoryUI *ui) {
  if (!ui)
    return;
  ui->active = false;
  ui->character = NULL;
  ui->selected_equip = -1;
  ui->selected_inv = -1;
}

static const char *slot_type_name(EquipSlotType type) {
  switch (type) {
  case EQUIP_SLOT_ARMOR:
    return "Armor";
  case EQUIP_SLOT_WEAPON_MELEE:
    return "Weapon";
  case EQUIP_SLOT_TRINKET:
    return "Trinket";
  default:
    return "Slot";
  }
}

bool inventory_ui_update(InventoryUI *ui) {
  if (!ui || !ui->active)
    return false;

  bool mouse_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
  bool mouse_pressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  int ec = ui->character ? ui->character->equipment_count : 0;

  for (int i = 0; i < ec; i++)
    button_update(&ui->equip_buttons[i], mouse_down);
  for (int i = 0; i < ec; i++) {
    if (ui->equip_buttons[i].pressed && mouse_pressed) {
      if (ui->selected_inv >= 0) {
        Item *it = ui->character->inventory.items[ui->selected_inv];
        if (it && i < ec && item_can_equip_in_slot(
                                it->type, ui->character->equipment[i].type)) {
          Item *prev = character_equip_item(ui->character, it, i);
          ui->character->inventory.items[ui->selected_inv] = prev;
          if (prev)
            snprintf(ui->message, sizeof(ui->message),
                     "Swapped %s for %s", prev->name, it->name);
          else
            snprintf(ui->message, sizeof(ui->message), "Equipped %s",
                     it->name);
        } else {
          snprintf(ui->message, sizeof(ui->message), "Cannot equip there");
        }
        ui->selected_inv = -1;
        ui->selected_equip = -1;
      } else {
        ui->selected_equip = i;
        ui->selected_inv = -1;
        ui->message[0] = '\0';
      }
    }
  }

  for (int i = 0; i < INVENTORY_SIZE; i++)
    button_update(&ui->inv_buttons[i], mouse_down);
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (ui->inv_buttons[i].pressed && mouse_pressed) {
      if (ui->selected_equip >= 0) {
        int ei = ui->selected_equip;
        if (ei < ec && ui->character->equipment[ei].item != NULL) {
          if (ui->character->inventory.items[i] == NULL) {
            Item *prev = character_unequip_item(ui->character, ei);
            ui->character->inventory.items[i] = prev;
            snprintf(ui->message, sizeof(ui->message), "Unequipped %s",
                     prev->name);
          } else {
            snprintf(ui->message, sizeof(ui->message),
                     "Inventory slot full");
          }
        }
        ui->selected_equip = -1;
        ui->selected_inv = -1;
      } else {
        ui->selected_inv = i;
        ui->selected_equip = -1;
        ui->message[0] = '\0';
      }
    }
  }

  button_update(&ui->destroy_button, mouse_down);
  if (ui->destroy_button.pressed && mouse_pressed) {
    if (ui->selected_inv >= 0) {
      Item *it = ui->character->inventory.items[ui->selected_inv];
      if (it) {
        snprintf(ui->message, sizeof(ui->message), "Destroyed %s",
                 it->name);
        item_free(it);
        ui->character->inventory.items[ui->selected_inv] = NULL;
      }
      ui->selected_inv = -1;
    } else {
      snprintf(ui->message, sizeof(ui->message),
               "Select an inventory item");
    }
  }

  button_update(&ui->close_button, mouse_down);
  if (ui->close_button.pressed && mouse_pressed) {
    ui->active = false;
    return false;
  }
  return true;
}

void inventory_ui_render(InventoryUI *ui) {
  if (!ui || !ui->active)
    return;

  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  DrawRectangle(0, 0, sw, sh, ui->overlay_color);
  DrawRectangleRec(ui->bounds, ui->bg_color);
  DrawRectangleLinesEx(ui->bounds, 4, BLACK);

  const char *title = "INVENTORY";
  int title_w = MeasureText(title, 32);
  DrawText(title, ui->bounds.x + (ui->bounds.width - title_w) / 2,
           ui->bounds.y + 20, 32, BLACK);

  DrawText("EQUIPMENT", ui->bounds.x + COL1_X, ui->bounds.y + 55, 20,
           DARKGRAY);
  DrawText("INVENTORY", ui->bounds.x + COL2_X, ui->bounds.y + 55, 20,
           DARKGRAY);

  int ec = ui->character ? ui->character->equipment_count : 0;
  char label[64];

  for (int i = 0; i < ec; i++) {
    Button *b = &ui->equip_buttons[i];
    EquipSlot *slot = &ui->character->equipment[i];
    if (slot->item)
      snprintf(label, sizeof(label), "%s: %s", slot_type_name(slot->type),
               slot->item->name);
    else
      snprintf(label, sizeof(label), "%s: Empty",
               slot_type_name(slot->type));
    button_set_label(b, label, 18, BLACK);
    button_draw(b);
    if (ui->selected_equip == i) {
      DrawRectangleLines(b->x - 3, b->y - 3, b->width + 6, b->height + 6,
                         YELLOW);
    }
  }

  for (int i = 0; i < INVENTORY_SIZE; i++) {
    Button *b = &ui->inv_buttons[i];
    Item *it = ui->character->inventory.items[i];
    if (it)
      snprintf(label, sizeof(label), "%s", it->name);
    else
      snprintf(label, sizeof(label), "Empty");
    button_set_label(b, label, 18, BLACK);
    button_draw(b);
    if (ui->selected_inv == i) {
      DrawRectangleLines(b->x - 3, b->y - 3, b->width + 6, b->height + 6,
                         YELLOW);
    }
  }

  button_draw(&ui->destroy_button);
  button_draw(&ui->close_button);

  if (ui->message[0] != '\0') {
    DrawText(ui->message, ui->bounds.x + COL1_X,
             ui->bounds.y + INV_H - 35, 18, DARKGRAY);
  }
}
