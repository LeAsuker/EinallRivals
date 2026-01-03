#include "game/actions.h"
#include <stdlib.h>
#include <string.h>


void skill_free(Skill *skill) {
    if (skill == NULL) return;
    if (skill->area_of_effect) {
        free(skill->area_of_effect);
        skill->area_of_effect = NULL;
        skill->aoe_size = 0;
    }
}

Skill spear_strike = {
    .name = "Spear Strike",
    .id = 101,
    .damage = -1, // Damage to be set dynamically
    .is_magic = false,
    .cooldown = 0,
    .range = 1, 
    .area_of_effect = NULL,
    .aoe_size = 0,
    .icon = {0}
};

void action_copy_spear_strike(Skill *dest_skill) {
    if (dest_skill == NULL) return;
    memcpy(dest_skill, &spear_strike, sizeof(Skill));
    return;
}

Skill bite = {
    .name = "Bite",
    .id = 102,
    .damage = -1, // Damage to be set dynamically
    .is_magic = false,
    .cooldown = 0,
    .range = 1, 
    .area_of_effect = NULL,
    .aoe_size = 0,
    .icon = {0}
};

void action_copy_bite(Skill *dest_skill) {
    if (dest_skill == NULL) return;
    memcpy(dest_skill, &bite, sizeof(Skill));
    return;
}

void action_set_damage(Skill *skill, Actor *owner) {
    if (skill == NULL) return;
    if (owner == NULL) return;

    if (skill->is_magic) {
        // Example: magic damage scales with magic attack
        int damage = owner->magic_attack;
        skill->damage = damage;
        return;
    }
    skill->damage = owner->phys_attack;
}

void action_add_skill_to_actor(Actor *actor, Skill *skill) {
    if (actor == NULL || skill == NULL) return;
    if (actor->skill_count >= 5) return; // Max skills reached

    actor->skills[actor->skill_count] = *skill;
    actor->skill_count++;
}