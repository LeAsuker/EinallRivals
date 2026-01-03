#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

void action_copy_spear_strike(Skill *dest_skill);
void action_copy_bite(Skill *dest_skill);
void skill_free(Skill *skill);
void action_set_damage(Skill *skill, Actor *owner);
void action_add_skill_to_actor(Actor *actor, Skill *skill);
// Asset lifecycle for action icons
void actions_load_icons(void);
void actions_unload_icons(void);

#endif