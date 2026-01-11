#include "game/actor.h"
#include "game/actions.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Default Unit Class Definitions
// ============================================================================

UnitClass CLASS_MILITIA = {
    .name = "Militia",
    .max_health = 20,
    .movement = 4,
    .phys_attack = 8,
    .phys_defense = 3,
    .magic_attack = 0,
    .magic_defense = 3,
    .luck = 1,
    .class_tree = { .promotions = {NULL, NULL, NULL, NULL}, .promotion_count = 0 }
};

UnitClass CLASS_WARG = {
    .name = "Warg",
    .max_health = 16,
    .movement = 3,
    .phys_attack = 7,
    .phys_defense = 2,
    .magic_attack = 1,
    .magic_defense = 1,
    .luck = 0,
    .class_tree = { .promotions = {NULL, NULL, NULL, NULL}, .promotion_count = 0 }
};

UnitClass CLASS_SPEARMAN = {
    .name = "Spearman",
    .max_health = 25,
    .movement = 4,
    .phys_attack = 10,
    .phys_defense = 7,
    .magic_attack = 0,
    .magic_defense = 6,
    .luck = 2,
    .class_tree = { .promotions = {NULL, NULL, NULL, NULL}, .promotion_count = 0 }
};

UnitClass CLASS_SWORDSMAN = {
    .name = "Swordsman",
    .max_health = 22,
    .movement = 4,
    .phys_attack = 12,
    .phys_defense = 5,
    .magic_attack = 0,
    .magic_defense = 4,
    .luck = 3,
    .class_tree = { .promotions = {NULL, NULL, NULL, NULL}, .promotion_count = 0 }
};

// ============================================================================
// Class Getters
// ============================================================================

UnitClass *class_get_militia(void) {
    return &CLASS_MILITIA;
}

UnitClass *class_get_warg(void) {
    return &CLASS_WARG;
}

// ============================================================================
// Genetics and Veterancy Initialization
// ============================================================================

void genetics_init(Genetics *genetics) {
    if (genetics == NULL) return;
    genetics->max_health = rand() % 6;    // 0-5
    genetics->movement = rand() % 6;
    genetics->phys_attack = rand() % 6;
    genetics->phys_defense = rand() % 6;
    genetics->magic_attack = rand() % 6;
    genetics->magic_defense = rand() % 6;
    genetics->luck = rand() % 6;
}

void veterancy_init(Veterancy *veterancy) {
    if (veterancy == NULL) return;
    veterancy->max_health = 0;
    veterancy->movement = 0;
    veterancy->phys_attack = 0;
    veterancy->phys_defense = 0;
    veterancy->magic_attack = 0;
    veterancy->magic_defense = 0;
    veterancy->luck = 0;
}

// ============================================================================
// Stats Calculation
// ============================================================================

Stats character_get_stats(Character *character) {
    Stats stats = {0};
    if (character == NULL) return stats;
    
    UnitClass *cls = character->unit_class;
    Genetics *gen = &character->genetics;
    Veterancy *vet = &character->veterancy;
    
    if (cls != NULL) {
        stats.max_health = cls->max_health + gen->max_health + vet->max_health;
        stats.movement = cls->movement + gen->movement + vet->movement;
        stats.phys_attack = cls->phys_attack + gen->phys_attack + vet->phys_attack;
        stats.phys_defense = cls->phys_defense + gen->phys_defense + vet->phys_defense;
        stats.magic_attack = cls->magic_attack + gen->magic_attack + vet->magic_attack;
        stats.magic_defense = cls->magic_defense + gen->magic_defense + vet->magic_defense;
        stats.luck = cls->luck + gen->luck + vet->luck;
    }
    
    return stats;
}

// ============================================================================
// Character Creation and Initialization
// ============================================================================

Character *militia_create(Faction *owner, Texture2D sprite) {
    Character *character = malloc(sizeof(Character));
    if (character == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for character\n");
        return NULL;
    }
    
    militia_init(character, owner, sprite);
    return character;
}

Character *character_create_from_class(Faction *owner, Texture2D sprite, UnitClass *unit_class) {
    Character *character = malloc(sizeof(Character));
    if (character == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for character\n");
        return NULL;
    }
    character_init_from_class(character, owner, sprite, unit_class);
    return character;
}

void militia_init(Character *character, Faction *owner, Texture2D sprite) {
    NULL_CHECK_VOID(character);
    NULL_CHECK_VOID(owner);
    character->sprite = sprite;
    character->owner = owner;
    
    // Initialize action flags
    character->can_move = true;
    character->can_act = true;
    
    // Initialize level and experience
    character->level = 1;
    character->next_level_xp = 100;
    character->level_up_pending = false;
    
    // Set class reference
    character->unit_class = &CLASS_MILITIA;
    
    // Initialize genetics (random) and veterancy (zero)
    genetics_init(&character->genetics);
    veterancy_init(&character->veterancy);
    
    // Set current health to max
    Stats stats = character_get_stats(character);
    character->curr_health = stats.max_health;
    
    // Copy class name as character name
    strcpy(character->name, CLASS_MILITIA.name);
    
    // Initialize skills
    character->skill_count = 0;
    {
        Skill tmp;
        action_copy_loot(&tmp);
        action_add_skill_to_character(character, &tmp);
        action_copy_spear_strike(&tmp);
        action_add_skill_to_character(character, &tmp);
    }
}

void warg_init(Character *character, Faction *owner, Texture2D sprite) {
    NULL_CHECK_VOID(character);
    NULL_CHECK_VOID(owner);
    character->sprite = sprite;
    character->owner = owner;
    
    // Initialize action flags
    character->can_move = true;
    character->can_act = true;
    
    // Initialize level and experience
    character->level = 1;
    character->next_level_xp = 100;
    character->level_up_pending = false;
    
    // Set class reference
    character->unit_class = &CLASS_WARG;
    
    // Initialize genetics (random) and veterancy (zero)
    genetics_init(&character->genetics);
    veterancy_init(&character->veterancy);
    
    // Set current health to max
    Stats stats = character_get_stats(character);
    character->curr_health = stats.max_health;
    
    // Copy class name as character name
    strcpy(character->name, CLASS_WARG.name);
    
    // Initialize skills
    character->skill_count = 0;
    {
        Skill tmp;
        action_copy_bite(&tmp);
        action_add_skill_to_character(character, &tmp);
    }
}

void character_init_from_class(Character *character, Faction *owner, 
                               Texture2D sprite, UnitClass *unit_class) {
    NULL_CHECK_VOID(character);
    NULL_CHECK_VOID(owner);
    NULL_CHECK_VOID(unit_class);
    if (strcmp(unit_class->name, "Militia") == 0) {
        militia_init(character, owner, sprite); // Add default militia skills
    } else if (strcmp(unit_class->name, "Warg") == 0) {
        warg_init(character, owner, sprite); // Add default warg skills
    } else {
        character->sprite = sprite;
        character->owner = owner;
        
        // Initialize action flags
        character->can_move = true;
        character->can_act = true;
        
        // Initialize level and experience
        character->level = 1;
        character->next_level_xp = 100;
        character->level_up_pending = false;
        
        // Set class reference
        character->unit_class = unit_class;
        
        // Initialize genetics (random) and veterancy (zero)
        genetics_init(&character->genetics);
        veterancy_init(&character->veterancy);
        
        // Set current health to max
        Stats stats = character_get_stats(character);
        character->curr_health = stats.max_health;
        
        // Copy class name as character name
        strcpy(character->name, unit_class->name);
        
        // Initialize skills (empty for generic classes)
        character->skill_count = 0;
    }
}

void character_free(Character *character) {
    if (character != NULL) {
        // Free any per-skill allocations
        for (int i = 0; i < character->skill_count; i++) {
            skill_free(&character->skills[i]);
        }
        free(character);
    }
}

// ============================================================================
// Character State Management
// ============================================================================

void character_reset_turn_flags(Character *character) {
    NULL_CHECK_VOID(character);
    character->can_move = true;
    character->can_act = true;
}

void character_end_turn(Character *character) {
    NULL_CHECK_VOID(character);
    character->can_move = false;
    character->can_act = false;
}

bool character_can_perform_action(Character *character) {
    return character->can_move || character->can_act;
}

bool character_is_alive(Character *character) {
    return character->curr_health > 0;
}

// ============================================================================
// Character Stats and Leveling
// ============================================================================

void character_take_damage(Character *character, int damage) {
    NULL_CHECK_VOID(character);
    character->curr_health -= damage;
    
    // Clamp health to 0 minimum
    if (character->curr_health < 0) {
        character->curr_health = 0;
    }
    
    // Log death
    if (!character_is_alive(character)) {
        printf("%s has been defeated!\n", character->name);
    }
}

void character_heal(Character *character, int amount) {
    NULL_CHECK_VOID(character);
    Stats stats = character_get_stats(character);
    character->curr_health += amount;
    
    // Clamp health to max
    if (character->curr_health > stats.max_health) {
        character->curr_health = stats.max_health;
    }
}

void character_gain_experience(Character *character, int xp) {
    NULL_CHECK_VOID(character);
    character->next_level_xp -= xp;

    // If we've reached or passed required XP, mark a pending level-up
    if (character->next_level_xp <= 0) {
        character->next_level_xp = 0;
        character->level_up_pending = true;
    }
}

void character_level_up(Character *character) {
    NULL_CHECK_VOID(character);
    character->level++;
    
    // Increase veterancy stats (permanent bonuses from leveling)
    character->veterancy.max_health += 3;
    character->veterancy.phys_attack += 1;
    character->veterancy.phys_defense += 1;
    character->veterancy.magic_attack += 1;
    character->veterancy.magic_defense += 1;
    
    // Full heal on level up
    Stats stats = character_get_stats(character);
    character->curr_health = stats.max_health;
    
    // Experience needed for next level (exponential growth)
    character->next_level_xp = 100 * character->level;
    // Clear pending flag when level-up is performed manually
    character->level_up_pending = false;
    printf("%s leveled up to level %d!\n", character->name, character->level);
}

// ============================================================================
// Character Queries
// ============================================================================

bool character_belongs_to_faction(Character *character, Faction *faction) {
    NULL_CHECK_RET(character, false);
    NULL_CHECK_RET(faction, false);
    return character->owner == faction;
}

bool character_is_enemy(Character *char1, Character *char2) {
    NULL_CHECK_RET(char1, false);
    NULL_CHECK_RET(char2, false);
    return char1->owner != char2->owner;
}

int character_get_health_percentage(Character *character) {
    NULL_CHECK_RET(character, 0);
    Stats stats = character_get_stats(character);
    if (stats.max_health == 0) return 0;
    return (character->curr_health * 100) / stats.max_health;
}

int character_get_max_skill_range(Character *character) {
    if (character == NULL) return 0;
    int max_range = 0;
    for (int i = 0; i < character->skill_count; i++) {
        if (character->skills[i].range > max_range) {
            max_range = character->skills[i].range;
        }
    }
    return max_range;
}

bool character_has_pending_level_up(Character *character) {
    return character->level_up_pending;
}

// ============================================================================
// Character Arrays and Groups
// ============================================================================

Character *character_array_create_from_class(int count, Faction *owner,
                                              Texture2D sprite, UnitClass *unit_class) {
    NULL_CHECK_RET(owner, NULL);
    NULL_CHECK_RET(unit_class, NULL);
    Character *characters = malloc(sizeof(Character) * count);
    
    if (characters == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for character array\n");
        return NULL;
    }
    
    // Initialize all characters
    for (int i = 0; i < count; i++) {
        // Init since we have an array, use init function directly
        character_init_from_class(&characters[i], owner, sprite, unit_class);
    }
    
    return characters;
}

void character_array_free(Character *characters, int count) {
    if (characters != NULL) {
        for (int i = 0; i < count; i++) {
            for (int s = 0; s < characters[i].skill_count; s++) {
                skill_free(&characters[i].skills[s]);
            }
        }
        free(characters);
    }
}

void character_array_reset_turns(Character *characters, int count) {
    for (int i = 0; i < count; i++) {
        if (character_is_alive(&characters[i])) {
            character_reset_turn_flags(&characters[i]);
        }
    }
}

int character_array_count_alive(Character *characters, int count) {
    int alive_count = 0;
    
    for (int i = 0; i < count; i++) {
        if (character_is_alive(&characters[i])) {
            alive_count++;
        }
    }
    
    return alive_count;
}
