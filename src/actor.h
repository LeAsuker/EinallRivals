#ifndef ACTOR_H_
#define ACTOR_H_

#include "types.h"
#include "raylib.h"
#include <stdbool.h>

// Actor creation and initialization
Actor *actor_create(Faction *owner, Texture2D sprite);
void actor_init(Actor *actor, Faction *owner, Texture2D sprite);
void actor_free(Actor *actor);

// Actor state management
void actor_reset_turn_flags(Actor *actor);
void actor_end_turn(Actor *actor);
bool actor_can_perform_action(Actor *actor);
bool actor_is_alive(Actor *actor);

// Actor stats and leveling
void actor_take_damage(Actor *actor, int damage);
void actor_heal(Actor *actor, int amount);
void actor_gain_experience(Actor *actor, int xp);
void actor_level_up(Actor *actor);

// Actor queries
bool actor_belongs_to_faction(Actor *actor, Faction *faction);
bool actor_is_enemy(Actor *actor1, Actor *actor2);
int actor_get_health_percentage(Actor *actor);

// Actor arrays and groups
Actor *actor_array_create(int count, Faction *owner, Texture2D sprite);
void actor_array_free(Actor *actors, int count);
void actor_array_reset_turns(Actor *actors, int count);
int actor_array_count_alive(Actor *actors, int count);

// Unit template system (for future expansion)
typedef struct {
    char name[10];
    int max_health;
    int movement;
    int attack;
    int defense;
    int magic_attack;
    int magic_defense;
    int range;
} ActorTemplate;

void actor_init_from_template(Actor *actor, Faction *owner, 
                              Texture2D sprite, ActorTemplate *template);

#endif