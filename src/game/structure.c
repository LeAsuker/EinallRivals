#include "game/structure.h"
#include <stdlib.h>
#include <string.h>

Structure *structure_create(Texture2D sprite, const char *name, bool passable, bool lootable) {
    Structure *s = malloc(sizeof(Structure));
    if (!s) return NULL;
    s->sprite = sprite;
    s->passable = passable;
    s->lootable = lootable;
    if (name) {
        strncpy(s->name, name, sizeof(s->name) - 1);
        s->name[sizeof(s->name) - 1] = '\0';
    } else {
        s->name[0] = '\0';
    }
    return s;
}

void structure_free(Structure *s) {
    if (!s) return;
    free(s);
}
