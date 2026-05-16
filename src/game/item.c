#include "game/item.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Item Type / Slot Compatibility
// ============================================================================

bool item_can_equip_in_slot(ItemType item_type, EquipSlotType slot_type) {
  switch (item_type) {
  case ITEM_TYPE_ARMOR:
    return slot_type == EQUIP_SLOT_ARMOR;
  case ITEM_TYPE_MELEE_WEAPON:
    return slot_type == EQUIP_SLOT_WEAPON_MELEE;
  case ITEM_TYPE_TRINKET:
    return slot_type == EQUIP_SLOT_TRINKET;
  case ITEM_TYPE_NONE:
  default:
    return false;
  }
}

// ============================================================================
// Item Lifecycle
// ============================================================================

Item *item_create(int id, const char *name, ItemType type, Stats stats) {
  Item *item = malloc(sizeof(Item));
  if (item == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for item\n");
    return NULL;
  }
  item->id = id;
  item->type = type;
  item->stats = stats;
  item->picture = (Texture2D){0};
  if (name != NULL) {
    strncpy(item->name, name, sizeof(item->name) - 1);
    item->name[sizeof(item->name) - 1] = '\0';
  } else {
    item->name[0] = '\0';
  }
  return item;
}

void item_free(Item *item) {
  NULL_CHECK_VOID(item);
  // Texture lifetime is managed externally (item atlas / sprite loader)
  free(item);
}

// ============================================================================
// Character Equipment Operations
// ============================================================================

Item *character_equip_item(Character *character, Item *item, int slot_index) {
  NULL_CHECK_RET(character, NULL);
  NULL_CHECK_RET(item, NULL);
  if (slot_index < 0 || slot_index >= character->equipment_count) {
    return NULL;
  }
  EquipSlot *slot = &character->equipment[slot_index];
  if (!item_can_equip_in_slot(item->type, slot->type)) {
    return NULL;
  }
  Item *prev = slot->item;
  slot->item = item;
  return prev;
}

Item *character_unequip_item(Character *character, int slot_index) {
  NULL_CHECK_RET(character, NULL);
  if (slot_index < 0 || slot_index >= character->equipment_count) {
    return NULL;
  }
  Item *prev = character->equipment[slot_index].item;
  character->equipment[slot_index].item = NULL;
  return prev;
}

// ============================================================================
// Character Inventory Operations
// ============================================================================

bool character_add_item_to_inventory(Character *character, Item *item) {
  NULL_CHECK_RET(character, false);
  NULL_CHECK_RET(item, false);
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (character->inventory.items[i] == NULL) {
      character->inventory.items[i] = item;
      return true;
    }
  }
  return false;
}

Item *character_remove_item_from_inventory(Character *character,
                                           int inventory_index) {
  NULL_CHECK_RET(character, NULL);
  if (inventory_index < 0 || inventory_index >= INVENTORY_SIZE) {
    return NULL;
  }
  Item *prev = character->inventory.items[inventory_index];
  character->inventory.items[inventory_index] = NULL;
  return prev;
}
