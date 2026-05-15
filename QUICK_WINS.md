# Quick Refactoring Wins

This is a curated map of low-risk, high-payoff refactors. Each entry has: the file, the smell, the fix, and the estimated risk. Tackle them in the order listed.

> Rule of thumb: if a change touches only one `.c` file and no headers, it is safe to do in one commit. If it changes a header, check consumers in the same module first.

---

## 1. Extract `character_init_base` — kill triplicated setup code

**File:** `src/game/actor.c`  
**Smell:** `militia_init`, `warg_init`, and the fallback branch in `character_init_from_class` are ~20-line copy-paste blocks that set flags, level, class, genetics, health, name, and skills.

**Fix:** Extract a private helper:

```c
static void character_init_base(Character *c, Faction *owner, Texture2D sprite,
                                  const UnitClass *cls, const ActionIcons *icons)
```

That sets the common fields. Then `militia_init` becomes:

```c
void militia_init(...) {
  character_init_base(character, owner, sprite, &CLASS_MILITIA, icons);
  // add militia-specific skills only
  action_copy_loot(&tmp); action_add_skill_to_character(...);
  action_copy_spear_strike(&tmp, icons); action_add_skill_to_character(...);
}
```

**Risk:** Very low. No header changes; purely internal helper.

---

## 2. Unroll the four-direction neighbor loop duplication

**File:** `src/game/map.c`  
**Smell:** `map_spread_terrain` and `calculate_range_recursive` both repeat this block four times:

```c
if ((neighbor = map_get_cell(..., x, y - 1)) != NULL) { ... }
// Down
// Left
// Right
```

**Fix:** Add a small macro or `static inline` helper in `map.c`:

```c
static inline void map_visit_cardinal(Point *map, GridConfig *grid, int x, int y,
                                      void (*fn)(Point *neighbor)) {
  Point *n;
  if ((n = map_get_cell(map, grid, x, y - 1))) fn(n);
  if ((n = map_get_cell(map, grid, x, y + 1))) fn(n);
  if ((n = map_get_cell(map, grid, x - 1, y))) fn(n);
  if ((n = map_get_cell(map, grid, x + 1, y))) fn(n);
}
```

Or keep it simple with a `#define FOR_CARDINAL(map, grid, x, y, n, body)` macro.

**Risk:** Very low. Purely local to `map.c`.

---

## 3. Extract grid rectangle helpers

**File:** `src/core/utils.c` (add), `src/render/rendering.c`, `src/input/input.c` (consume)  
**Smell:** This expression is copy-pasted in ~6 places:

```c
grid_config->grid_offset_x + grid_config->max_grid_cells_x * grid_config->grid_cell_size
```

Same for the Y variant.

**Fix:** Add inline helpers to `src/core/utils.h`:

```c
static inline int grid_right(const GridConfig *g) {
  return g->grid_offset_x + g->max_grid_cells_x * g->grid_cell_size;
}
static inline int grid_bottom(const GridConfig *g) { ... }
static inline int grid_width(const GridConfig *g) { ... }
static inline int grid_height(const GridConfig *g) { ... }
```

Replace the long arithmetic in `safe_mouse_x`, `safe_mouse_y`, `input_layout_buttons`, `render_ui`, `render_cell_info`, etc.

**Risk:** Low. Only touches `.c` files and a single header addition.

---

## 4. Collapse three flag-clearing loops into one helper

**File:** `src/game/map.c`  
**Smell:** `map_clear_range_flags`, `map_clear_movement_range_flags`, `map_clear_attack_range_flags` are nearly identical loops over the same array.

**Fix:** Keep the three public functions (they are in the header), but make them all call a single static helper:

```c
static void map_clear_flags_impl(Point *map, int total_cells,
                                 bool move, bool attack) {
  for (int i = 0; i < total_cells; i++) {
    if (move) map[i].in_range = false;
    if (attack) map[i].in_attack_range = false;
  }
}
```

**Risk:** Very low.

---

## 5. Extract AI target search helpers

**File:** `src/game/game_logic.c`  
**Smell:** `game_process_ai_turn` contains three almost-identical `for (int c = 0; c < total_cells; c++)` loops that scan the map for enemies. The function is ~200 lines and does too much.

**Fix:** Extract two static helpers inside `game_logic.c`:

```c
static Point *ai_find_closest_enemy_in_range(Point *map, int total_cells,
                                             Point *origin, Character *me,
                                             int max_range, int *out_dist);

static Point *ai_find_closest_enemy(Point *map, int total_cells,
                                  Point *origin, Character *me, int *out_dist);
```

Then `game_process_ai_turn` calls them instead of inlining the loops.

**Risk:** Low. Private to `.c` file, no header change.

---

## 6. Standardize null checks to `NULL_CHECK_*` macros

**File:** `src/game/actor.c`, `src/game/combat.c`, `src/game/actions.c`  
**Smell:** Some functions use the project `NULL_CHECK_VOID` / `NULL_CHECK_RET` macros; others use raw `if (x == NULL) return;`. Inconsistency makes automated grepping harder.

**Fix:** Search for `if (\w+ == NULL)` followed by `return` or `return ...;` in `src/game/*.c` and replace with the macros. Example in `actor.c`:

```c
// Before
void genetics_init(Genetics *genetics) {
  if (genetics == NULL) return;
  ...
}

// After
void genetics_init(Genetics *genetics) {
  NULL_CHECK_VOID(genetics);
  ...
}
```

**Risk:** Zero. Behavior-identical; uses existing macro.

---

## 7. Fix `map_all_8_neighs_terrain` — bug + bad comment

**File:** `src/game/map.c` (around line 306)  
**Smell:** The function checks all 8 neighbors but includes the center cell (offset 0,0). It also dereferences `cell->terrain.deep_version->id` without null-checking `deep_version`, which can segfault if deep_version is NULL (e.g. terrains with no deep variant). There is a leftover developer comment (`// wtf was this...`).

**Fix:**
1. Skip the center cell (`if (k == 1 && l == 1) continue;`).
2. Null-check `deep_version` before dereferencing:
   ```c
   if (cell->terrain.deep_version == NULL) return false;
   if (cell->terrain.deep_version->id == -1) return false;
   ```
3. Remove or rephrase the profane comment.

**Risk:** Low. Behavior fix; the function is only called during deep-terrain generation.

---

## 8. Add const-correctness to read-only parameters

**Files:** `src/game/map.c`, `src/game/combat.c`, `src/input/input.c`  
**Smell:** Many functions take `GridConfig *grid_config` or `Point *map` but never mutate them. Example:

```c
bool map_is_valid_coords(GridConfig *grid_config, int x, int y);
int combat_get_distance(Point *cell1, Point *cell2);
```

**Fix:** Change to `const GridConfig *` and `const Point *` where no mutation occurs. Start with the leaf helpers (no cascading changes):

- `map_is_valid_coords`
- `map_get_cell` (returns non-const pointer, but parameters can be const)
- `combat_get_distance`
- `input_is_mouse_over_map`
- `input_is_mouse_over_end_turn_button` (takes `const RenderContext *`)

**Risk:** Very low. Compile-time safety only. If any consumer passes a non-const pointer, C allows implicit conversion to const.

---

## 9. Consolidate `roll_hit` and `roll_crit`

**File:** `src/game/combat.c`  
**Smell:** `roll_hit` and `roll_crit` are identical `rand() % 100 < chance` blocks.

**Fix:** Replace both with one `static bool roll_chance(int chance)`.

**Risk:** Zero. No header change.

---

## 10. Remove duplicated comment lines in rendering

**File:** `src/render/rendering.c`  
**Smell:** Lines 46-47 have the same comment twice:

```c
// Draw all character stats and genetics markers in one call
// Draw all character stats and genetics markers in one call
```

**Fix:** Delete one line.

**Risk:** Zero.

---

## 11. Extract a single `grid_total_cells` helper

**File:** `src/game/map.c`, `src/game/game_logic.c`, `src/render/rendering.c`  
**Smell:** `grid_config->max_grid_cells_x * grid_config->max_grid_cells_y` is computed inline in at least 5 places.

**Fix:** Add to `src/core/utils.h`:

```c
static inline int grid_total_cells(const GridConfig *g) {
  return g->max_grid_cells_x * g->max_grid_cells_y;
}
```

Replace all inline multiplications.

**Risk:** Very low.

---

## 12. Replace hardcoded skill ID `103` with a named constant

**File:** `src/input/input.c` (around line 230)  
**Smell:** `if (s->id == 103) { // Loot skill special case... }` uses a magic number.

**Fix:** Add `#define SKILL_ID_LOOT 103` (or an enum value) in `src/game/actions.h` and use it in `input.c`.

**Risk:** Very low. One constant, one usage site.

---

## 13. Clean up `faction_init.c` with a table-driven initializer

**File:** `src/game/faction_init.c`  
**Smell:** `faction_init_default` manually assigns 6 fields for each of 3 factions in a row.

**Fix:** Use a small static const table:

```c
static const struct { const char *name; Color prim, sec; bool playable; } FACTION_DEFS[] = {
  {"Darkus", PURPLE, DARKGRAY, true},
  {"Ventus", GREEN,  WHITE,    true},
  {"Gaia",   BROWN,  BLACK,    false},
};
```

Then loop over the table to populate `factions[i]`. This makes adding a 4th faction a one-liner.

**Risk:** Low. Local to one function.

---

## 14. Make `render_ui` and `render_actions` layout less brittle

**File:** `src/render/rendering.c`  
**Smell:** Hardcoded multipliers like `ctx->grid_cell_size * 8`, `* 17`, `* 5` are scattered and make UI resizing fragile.

**Fix:** Add a small `UILayout` struct or `#define` block at the top of `rendering.c`:

```c
#define UI_PANEL_W_CELLS  8
#define UI_PANEL_H_CELLS 17
#define BUTTON_W_CELLS    8
#define BUTTON_H_CELLS    5
```

Then use them in `render_ui`, `render_actions`, `input_layout_buttons`, and `render_cell_info`.

**Risk:** Low. Named constants only.

---

## 15. Move `game_loop_playing` out of `main.c`

**File:** `src/main.c`  
**Smell:** `main()` is ~350 lines. The PLAYING mode block (initialization + the inner `while` loop) is ~200 lines and makes the entry point hard to scan.

**Fix:** Extract a static function inside `main.c`:

```c
static GameMode game_loop_playing(AppResources *app_res, ...);
```

It returns `GAME_MODE_MENU` or `GAME_MODE_QUIT`. This keeps `main()` focused on mode switching and `AppResources` registration.

**Risk:** Medium-low. No external linkage change, but careful with the `mode` variable lifetime.

---

## Quick-win priority order (suggested)

| # | Change | Files | Risk | Payoff |
|---|--------|-------|------|--------|
| 1 | `character_init_base` helper | `actor.c` | Very low | High (DRY) |
| 2 | Cardinal neighbor macro | `map.c` | Very low | Medium (DRY) |
| 3 | Grid rect helpers | `utils.h/c` + consumers | Low | High (readability) |
| 4 | `grid_total_cells` helper | `utils.h` + consumers | Very low | Medium |
| 5 | Collapse flag-clear loops | `map.c` | Very low | Low |
| 6 | `roll_hit`/`roll_crit` merge | `combat.c` | Zero | Low |
| 7 | Fix `map_all_8_neighs_terrain` | `map.c` | Low | High (bug fix) |
| 8 | Standardize null-check macros | `actor.c`, `combat.c`, `actions.c` | Zero | Medium (consistency) |
| 9 | Const-correctness sweep | `map.c`, `combat.c`, `input.c` | Very low | Medium |
| 10 | Extract AI search helpers | `game_logic.c` | Low | High (complexity) |
| 11 | Table-driven faction init | `faction_init.c` | Low | Medium |
| 12 | Magic number -> `SKILL_ID_LOOT` | `actions.h`, `input.c` | Very low | Low |
| 13 | UI layout constants | `rendering.c`, `input.c` | Low | Medium |
| 14 | Remove duplicate comment | `rendering.c` | Zero | Trivial |
| 15 | Extract `game_loop_playing` | `main.c` | Medium-low | High (clarity) |

---

## How to verify after each refactor

```bash
# 1. Build
make clean && make

# 2. Run (smoke test)
./bin/Debug/Grand-Grid-Dynasty
# Click through menu, move a unit, end a turn, open ESC menu.

# 3. If you want extra safety, run under valgrind (Linux)
valgrind --leak-check=full ./bin/Debug/Grand-Grid-Dynasty
```

Because the game is single-threaded and the map size is tiny (30x20), a 30-second manual smoke test covers 90%% of the execution paths touched by these refactors.
