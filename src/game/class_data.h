#ifndef CLASS_DATA_H_
#define CLASS_DATA_H_

#include "types.h"

// ============================================================================
// Unit class system
// ============================================================================

// Default unit class templates (statically allocated, shared across characters)
extern const UnitClass CLASS_MILITIA;
extern const UnitClass CLASS_WARG;
extern const UnitClass CLASS_SPEARMAN;
extern const UnitClass CLASS_SWORDSMAN;

/**
 * @brief Get a pointer to the global militia UnitClass template.
 * @return Pointer to the militia UnitClass.
 */
const UnitClass *class_get_militia(void);

/**
 * @brief Get a pointer to the global warg UnitClass template.
 * @return Pointer to the warg UnitClass.
 */
const UnitClass *class_get_warg(void);

/**
 * @brief Get a pointer to the global spearman UnitClass template.
 * @return Pointer to the spearman UnitClass.
 */
const UnitClass *class_get_spearman(void);

/**
 * @brief Get a pointer to the global swordsman UnitClass template.
 * @return Pointer to the swordsman UnitClass.
 */
const UnitClass *class_get_swordsman(void);

#endif
