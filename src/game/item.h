#ifndef ITEM_H_
#define ITEM_H_

#include "types.h"
#include <stdbool.h>

// ============================================================================
// Item Type System
// ============================================================================

/**
 * @brief Item types constraining which equipment slot an item can occupy.
 */
typedef enum {
  ITEM_TYPE_NONE = 0,
  ITEM_TYPE_ARMOR,
  ITEM_TYPE_MELEE_WEAPON,
  ITEM_TYPE_TRINKET,
  ITEM_TYPE_COUNT
} ItemType;

// ============================================================================
// Item Struct
// ============================================================================

/**
 * @brief An equippable item that grants stat bonuses to a character.
 *
 * Item picture textures are loaded and managed externally (e.g., by an item
 * atlas loader); item_free does NOT unload the Texture2D.
 */
struct Item {
  int id;               /**< Unique item identifier */
  char name[32];        /**< Human-readable item name */
  Texture2D picture;  /**< Item icon/texture (managed externally) */
  ItemType type;        /**< Slot compatibility constraint */
  Stats stats;          /**< Stat bonuses granted while equipped */
};

// ============================================================================
// Item Lifecycle
// ============================================================================

/**
 * @brief Check whether an item type can be equipped in a given slot type.
 * @param item_type The item's type.
 * @param slot_type The equipment slot type to test.
 * @return true if the item is compatible with the slot.
 */
bool item_can_equip_in_slot(ItemType item_type, EquipSlotType slot_type);

/**
 * @brief Allocate and initialize a new item.
 * @param id Item identifier.
 * @param name Human-readable name (copied, truncated if too long).
 * @param type Item type determining valid equipment slots.
 * @param stats Stat bonuses granted while equipped.
 * @return Newly allocated Item, or NULL on allocation failure.
 */
Item *item_create(int id, const char *name, ItemType type, Stats stats);

/**
 * @brief Free an item and any resources it owns.
 *
 * The item's picture Texture2D is NOT unloaded here; textures are managed
 * externally by the item atlas / sprite loader.
 *
 * @param item Item to free (NULL-safe).
 */
void item_free(Item *item);

// ============================================================================
// Character Equipment and Inventory Operations
// ============================================================================

/**
 * @brief Equip an item into a character's equipment slot if compatible.
 *
 * Replaces any previously equipped item in that slot. The old item is NOT
 * freed; it is returned for the caller to handle (e.g., move to inventory).
 *
 * @param character Character to equip.
 * @param item Item to equip.
 * @param slot_index Index into the character's equipment array.
 * @return The previously equipped item, or NULL if none was present.
 *         Returns NULL and does nothing if slot is invalid or incompatible.
 */
Item *character_equip_item(Character *character, Item *item, int slot_index);

/**
 * @brief Unequip an item from a character's equipment slot.
 * @param character Character to modify.
 * @param slot_index Index into the character's equipment array.
 * @return The previously equipped item, or NULL if empty/invalid.
 */
Item *character_unequip_item(Character *character, int slot_index);

/**
 * @brief Add an item to the character's inventory if a free slot exists.
 * @param character Character to receive the item.
 * @param item Item to store.
 * @return true if added, false if inventory is full.
 */
bool character_add_item_to_inventory(Character *character, Item *item);

/**
 * @brief Remove an item from the character's inventory by index.
 * @param character Character to modify.
 * @param inventory_index Index into the inventory array.
 * @return The removed item, or NULL if invalid/empty.
 */
Item *character_remove_item_from_inventory(Character *character,
                                           int inventory_index);

#endif
