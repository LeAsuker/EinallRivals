#ifndef FACTION_DATA_H_
#define FACTION_DATA_H_

#include "types.h"

// ============================================================================
// Faction identifiers
// ============================================================================

typedef enum {
  DARKUS = 0,
  VENTUS,
  GAIA
} Factions;

#define FACTION_COUNT 3

// ============================================================================
// Faction definition template (immutable configuration)
// ============================================================================

typedef struct {
  const char *name;
  Color prim_color;
  Color sec_color;
  bool playable;
  bool has_turn;
} FactionDef;

// ============================================================================
// Default faction definitions
// ============================================================================

extern const FactionDef FACTION_DEF_DARKUS;
extern const FactionDef FACTION_DEF_VENTUS;
extern const FactionDef FACTION_DEF_GAIA;

/**
 * @brief Get the definition template for a given faction index.
 *
 * @param faction Faction identifier.
 * @return Pointer to the static definition, or NULL if out of range.
 */
const FactionDef *faction_get_def(Factions faction);

/**
 * @brief Return the number of predefined faction definitions.
 * @return FACTION_COUNT.
 */
int faction_definition_count(void);

#endif
