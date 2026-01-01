#include "actor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Default stats for a basic militia unit
static const ActorTemplate DEFAULT_MILITIA = {
    .name = "Militia",
    .max_health = 20,
    .movement = 4,
    .attack = 8,
    .defense = 3,
    .magic_attack = 2,
    .magic_defense = 3,
    .range = 1
};

// ============================================================================
// Actor Creation and Initialization
// ============================================================================

Actor *actor_create(Faction *owner, Texture2D sprite) {
    Actor *actor = malloc(sizeof(Actor));
    if (actor == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for actor\n");
        return NULL;
    }
    
    actor_init(actor, owner, sprite);
    return actor;
}

void actor_init(Actor *actor, Faction *owner, Texture2D sprite) {
    actor->sprite = sprite;
    actor->owner = owner;
    
    // Initialize action flags
    actor->can_move = true;
    actor->can_act = true;
    
    // Initialize level and experience
    actor->level = 1;
    actor->next_level_xp = 100;
    
    // Initialize stats from default template
    strcpy(actor->name, DEFAULT_MILITIA.name);
    actor->max_health = DEFAULT_MILITIA.max_health;
    actor->curr_health = DEFAULT_MILITIA.max_health;
    actor->movement = DEFAULT_MILITIA.movement;
    actor->attack = DEFAULT_MILITIA.attack;
    actor->defense = DEFAULT_MILITIA.defense;
    actor->magic_attack = DEFAULT_MILITIA.magic_attack;
    actor->magic_defense = DEFAULT_MILITIA.magic_defense;
    actor->range = DEFAULT_MILITIA.range;
}

void actor_init_from_template(Actor *actor, Faction *owner, 
                              Texture2D sprite, ActorTemplate *template) {
    actor->sprite = sprite;
    actor->owner = owner;
    
    actor->can_move = true;
    actor->can_act = true;
    
    actor->level = 1;
    actor->next_level_xp = 100;
    
    // Use template stats
    strcpy(actor->name, template->name);
    actor->max_health = template->max_health;
    actor->curr_health = template->max_health;
    actor->movement = template->movement;
    actor->attack = template->attack;
    actor->defense = template->defense;
    actor->magic_attack = template->magic_attack;
    actor->magic_defense = template->magic_defense;
    actor->range = template->range;
}

void actor_free(Actor *actor) {
    if (actor != NULL) {
        free(actor);
    }
}

// ============================================================================
// Actor State Management
// ============================================================================

void actor_reset_turn_flags(Actor *actor) {
    actor->can_move = true;
    actor->can_act = true;
}

void actor_end_turn(Actor *actor) {
    actor->can_move = false;
    actor->can_act = false;
}

bool actor_can_perform_action(Actor *actor) {
    return actor->can_move || actor->can_act;
}

bool actor_is_alive(Actor *actor) {
    return actor->curr_health > 0;
}

// ============================================================================
// Actor Stats and Leveling
// ============================================================================

void actor_take_damage(Actor *actor, int damage) {
    actor->curr_health -= damage;
    
    // Clamp health to 0 minimum
    if (actor->curr_health < 0) {
        actor->curr_health = 0;
    }
    
    // Log death
    if (!actor_is_alive(actor)) {
        printf("%s has been defeated!\n", actor->name);
    }
}

void actor_heal(Actor *actor, int amount) {
    actor->curr_health += amount;
    
    // Clamp health to max
    if (actor->curr_health > actor->max_health) {
        actor->curr_health = actor->max_health;
    }
}

void actor_gain_experience(Actor *actor, int xp) {
    actor->next_level_xp -= xp;
    
    // Check for level up
    while (actor->next_level_xp <= 0) {
        actor_level_up(actor);
    }
}

void actor_level_up(Actor *actor) {
    actor->level++;
    
    // Stat increases (basic formula, can be expanded)
    actor->max_health += 3;
    actor->curr_health = actor->max_health; // Full heal on level up
    actor->attack += 1;
    actor->defense += 1;
    actor->magic_attack += 1;
    actor->magic_defense += 1;
    
    // Experience needed for next level (exponential growth)
    actor->next_level_xp = 100 * actor->level;
    
    printf("%s leveled up to level %d!\n", actor->name, actor->level);
}

// ============================================================================
// Actor Queries
// ============================================================================

bool actor_belongs_to_faction(Actor *actor, Faction *faction) {
    return actor->owner == faction;
}

bool actor_is_enemy(Actor *actor1, Actor *actor2) {
    return actor1->owner != actor2->owner;
}

int actor_get_health_percentage(Actor *actor) {
    if (actor->max_health == 0) return 0;
    return (actor->curr_health * 100) / actor->max_health;
}

// ============================================================================
// Actor Arrays and Groups
// ============================================================================

Actor *actor_array_create(int count, Faction *owner, Texture2D sprite) {
    Actor *actors = malloc(sizeof(Actor) * count);
    
    if (actors == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for actor array\n");
        return NULL;
    }
    
    // Initialize all actors
    for (int i = 0; i < count; i++) {
        actor_init(&actors[i], owner, sprite);
    }
    
    return actors;
}

void actor_array_free(Actor *actors, int count) {
    (void)count; // Unused parameter, but kept for API consistency
    if (actors != NULL) {
        free(actors);
    }
}

void actor_array_reset_turns(Actor *actors, int count) {
    for (int i = 0; i < count; i++) {
        if (actor_is_alive(&actors[i])) {
            actor_reset_turn_flags(&actors[i]);
        }
    }
}

int actor_array_count_alive(Actor *actors, int count) {
    int alive_count = 0;
    
    for (int i = 0; i < count; i++) {
        if (actor_is_alive(&actors[i])) {
            alive_count++;
        }
    }
    
    return alive_count;
}
