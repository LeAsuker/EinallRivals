#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

void action_copy_spear_strike(Skill *dest_skill);
void action_copy_bite(Skill *dest_skill);
void action_copy_loot(Skill *dest_skill);
void skill_free(Skill *skill);
void action_set_damage(Skill *skill, Actor *owner);
void action_add_skill_to_actor(Actor *actor, Skill *skill);
// Asset lifecycle for action icons
void actions_load_icons(void);
void actions_unload_icons(void);

// Execute a skill from attacker_cell onto defender_cell. This applies damage
// based on the Skill and the attacker's stats, grants experience, and updates
// unit/action state.
void execute_skill_at_cells(GridConfig *grid_config, Point *map, Point *attacker_cell, Point *defender_cell, Skill *skill);
void execute_loot_at_cells(GridConfig *grid_config, Point *map, Point *looter_cell, Point *lootable, Skill *skill);
#endif