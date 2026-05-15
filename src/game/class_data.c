#include "game/class_data.h"

// ============================================================================
// Default Unit Class Definitions
// ============================================================================

const UnitClass CLASS_MILITIA = {
    .name = "Militia",
    .max_health = 20,
    .movement = 4,
    .phys_attack = 8,
    .phys_defense = 3,
    .magic_attack = 0,
    .magic_defense = 3,
    .luck = 1,
    .class_tree = {.promotions = {NULL, NULL, NULL, NULL},
                   .promotion_count = 0}};

const UnitClass CLASS_WARG = {.name = "Warg",
                        .max_health = 16,
                        .movement = 3,
                        .phys_attack = 7,
                        .phys_defense = 2,
                        .magic_attack = 1,
                        .magic_defense = 1,
                        .luck = 0,
                        .class_tree = {.promotions = {NULL, NULL, NULL, NULL},
                                       .promotion_count = 0}};

const UnitClass CLASS_SPEARMAN = {
    .name = "Spearman",
    .max_health = 25,
    .movement = 4,
    .phys_attack = 10,
    .phys_defense = 7,
    .magic_attack = 0,
    .magic_defense = 6,
    .luck = 2,
    .class_tree = {.promotions = {NULL, NULL, NULL, NULL},
                   .promotion_count = 0}};

const UnitClass CLASS_SWORDSMAN = {
    .name = "Swordsman",
    .max_health = 22,
    .movement = 4,
    .phys_attack = 12,
    .phys_defense = 5,
    .magic_attack = 0,
    .magic_defense = 4,
    .luck = 3,
    .class_tree = {.promotions = {NULL, NULL, NULL, NULL},
                   .promotion_count = 0}};

// ============================================================================
// Class Getters
// ============================================================================

const UnitClass *class_get_militia(void) { return &CLASS_MILITIA; }

const UnitClass *class_get_warg(void) { return &CLASS_WARG; }

const UnitClass *class_get_spearman(void) { return &CLASS_SPEARMAN; }

const UnitClass *class_get_swordsman(void) { return &CLASS_SWORDSMAN; }
