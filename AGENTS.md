# Agent Instructions: Grand-Grid-Dynasty (Einall Rivals)

This document is the canonical reference for AI coding agents working in this repository. Read this file before making changes.

## Project Identity

- **Name**: Grand-Grid-Dynasty (also called Einall Rivals)
- **Type**: Tactical RPG (Fire Emblem-like), single-binary desktop game
- **Language**: C17
- **Framework**: raylib (graphics, input, audio, windowing)
- **Platforms**: Linux, macOS (Windows build files exist but are secondary)
- **License**: Not specified; treat as private codebase

## Quick Start (Build & Run)

```bash
# One-shot build and run (Linux)
./BuildAndLaunch.sh linux

# Or manual steps:
cd build && ./premake5 gmake
cd ..
make                    # Debug build
make config=release_x64 # Release build

# Binary location
./bin/Debug/Grand-Grid-Dynasty
```

- The build system auto-downloads raylib into `external/raylib-master` on first compile.
- The executable expects a `resources/` directory relative to its working directory (handled by `SearchAndSetResourceDir`).

## Architecture Overview

The game is a single-threaded C application with a classic game-loop architecture:

```
+----------------------------------+
| main.c                           |
|  - Window init (raylib)          |
|  - Mode switcher: MENU <-> PLAY  |
|  - Game loop at 60 FPS           |
+----------------------------------+
              |
    +---------+---------+
    |                   |
+---v---+          +----v-----------+
| Menu  |          | Playing        |
| (UI)  |          |  - Input       |
+-------+          |  - Game Logic  |
                   |  - Combat      |
                   |  - Rendering   |
                   +----------------+
```

### Module Layers (bottom to top)

| Layer | Directory | Responsibility |
|-------|-----------|----------------|
| **Types** | `src/types.h` | Core data structures shared by all modules. Changing a struct here cascades everywhere. |
| **Core** | `src/core/` | Low-level utilities: grid math, mouse clamping, cell flag flushing. No game logic. |
| **Game** | `src/game/` | Domain logic: actors, combat, map generation, terrain, structures, factions, turns, skills. |
| **Input** | `src/input/` | Mouse/keyboard handling, button hit-testing, action selection, movement commands. |
| **Render** | `src/render/` | Drawing: map tiles, units, structures, UI chrome, debug overlays. Reads state; never mutates game rules. |
| **UI** | `src/ui/` | Reusable UI primitives: Button, Modal, Menu. |

## Module Directory Reference

### `src/types.h` — The Shared Contract
This is the most critical file. It defines:
- `Character` (unit instance with stats, skills, genetics, veterancy, archetype, equipment)
- `Faction` (owning faction with color theme and character array)
- `Terrain` / `Point` (map cell: position, occupant, terrain, structure)
- `Skill` (action definition with AoE, range, cooldown, icon)
- `UnitClass` / `ClassTree` (class templates and promotion tree)
- `Archetype`, `EquipSlot`, `EquipSlotType` (archetype & equipment system)
- `Stats`, `Genetics`, `Veterancy` (stat calculation layers)
- `GridConfig` (pixel offsets and cell counts)
- `NULL_CHECK_VOID` / `NULL_CHECK_RET` macros (null-check guards used everywhere)

**Rule**: Do not change struct layouts here without checking every consumer.

### `src/core/` — Utilities
- `utils.c/h`: Grid allocation, safe mouse coordinate clamping, cell flag flushing.
- **Dependency policy**: Only depends on `types.h`.

### `src/game/` — Game Domain
All `.c` files in this directory depend on `types.h` and often on each other. There is no strict sub-layering; read headers before adding new cross-module calls.

| File | Domain | Key APIs |
|------|--------|----------|
| `actor.c/h` | Character lifecycle, stats, leveling, archetype & class system | `militia_create`, `character_get_stats`, `character_array_create_from_class`, `character_level_up`, `class_get_militia` |
| `actions.c/h` | Skill definitions, action icons, skill execution | `action_copy_spear_strike`, `execute_skill_at_cells`, `action_icons_load/unload` |
| `combat.c/h` | Combat execution, forecast, damage formulas, XP | `combat_execute`, `combat_forecast`, `combat_calculate_damage`, `combat_grant_experience` |
| `game_logic.c/h` | Turn state machine, victory checks, AI turns | `game_state_create`, `game_next_turn`, `game_process_ai_turn`, `game_check_victory_conditions` |
| `map.c/h` | Map array allocation, biome generation, path/range | `map_create`, `map_generate_all_biomes`, `map_calculate_movement_range`, `map_get_cell` |
| `terrain.c/h` | Terrain texture loading, passability | `terrain_init_all`, `terrain_unload_all`, `TerrainType` enum |
| `structure.c/h` | Structure data type (lairs, huts) | `structure_create`, `structure_free` |
| `structure_generation.c/h` | Procedural structure placement, Gaia spawning | `structure_generation_place_warg_lairs`, `structure_generation_spawn_wargs_around_lairs` |
| `spawning.c/h` | Faction troop placement into map corners | `spawning_place_faction_in_corner` |
| `faction_init.c/h` | Faction setup (DARKUS, VENTUS, GAIA) | `faction_init_default`, `factions_free_actors` |
| `biome_config.c/h` | Biome parameters (core count, spread range) | `biome_config_get_default` |
| `archetype.c/h` | Archetype slot configs and base-class lookup | `archetype_get_slot_config`, `archetype_get_base_class` |
| `cleanup.h` | `AppResources` bundle + atexit cleanup | `app_resources_register`, `app_resources_cleanup_atexit` |
| `raylib_check.c/h` | Raylib version compatibility checks | (rarely touched) |

### `src/input/` — Input Handling
- `input.c/h`: Frame-by-frame input update, mouse-to-cell mapping, button clicks, action button handling, end-turn button.
- Depends on: `types.h`, `game/actions.h`, `game/combat.h`, `ui/button.h`
- **Rule**: Input translates user actions into game state mutations. It may call `execute_skill_at_cells` and `game_end_current_turn`, but it should not contain game-rule logic (damage formulas, victory checks, etc.).

### `src/render/` — Presentation
- `rendering.c/h`: Main game rendering (`render_game`), debug overlays, cell info panels.
- `unit_sprites.c/h`: Unit texture loading/unloading.
- `structure_sprites.h`: Structure texture loading.
- **Rule**: Render code reads game state and draws it. It does not modify `Point` occupants, `Character` health, or `GameState` phase. If you need to change state during rendering, move that logic to `input/` or `game/`.

### `src/ui/` — UI Primitives
- `button.h`: `Button` struct and inline helpers (`button_init`, `button_set_rect`, `button_update`, `button_draw`).
- `modal.c/h`: Modal dialog system (ESC menu, level-up class choice).
- `menu.c/h`: Title screen menu (Start / Quit).
- **Policy**: UI modules depend only on `types.h`, `button.h`, and raylib. They do not depend on game logic directly.

## Data Flow & Ownership

### Initialization Flow (PLAYING mode entry)
```
main()
  -> grid_init()                 [core/utils]
  -> terrain_init_all()          [game/terrain]
  -> biome_config_get_default()  [game/biome_config]
  -> map_create() / map_generate_all_biomes()  [game/map]
  -> render_init()               [render/rendering]
  -> faction_init_default()      [game/faction_init]
  -> unit_sprites_load()         [render/unit_sprites]
  -> structure_sprites_load()    [render/structure_sprites]
  -> action_icons_load()         [game/actions]
  -> character_array_create_from_class() [game/actor]  (pass Archetype, base class auto-resolved)
  -> spawning_place_faction_in_corner()  [game/spawning]
  -> structure_generation_place_*()      [game/structure_generation]
  -> game_state_create()         [game/game_logic]
  -> modal_create()              [ui/modal]
  -> input_init() / input_layout_buttons() [input/input]
```

### Per-Frame Flow
```
main loop
  -> input_update()              [input/input]
  -> input_handle_action_click() [input/input]
  -> input_handle_left_click()   [input/input]
  -> game_process_ai_turn()      [game/game_logic]  (if AI turn)
  -> game_end_current_turn()     [game/game_logic]  (if requested)
  -> render_game()               [render/rendering]
```

### Resource Ownership & Cleanup
All resources allocated when entering PLAYING mode are tracked in the `AppResources` bundle (`src/game/cleanup.h`). The bundle is registered with `atexit()` so that any fatal exit path still frees memory and textures.

**Ownership table:**

| Resource | Created by | Freed by | Notes |
|----------|-----------|----------|-------|
| `Point *mapArr` | `map_create` | `map_free` | Lives in `AppResources` |
| `Character *` arrays | `character_array_create_from_class` | `character_array_free` | Called via `factions_free_actors` |
| `Faction` structs | `faction_init_default` | `factions_free_actors` | Factions array is stack-allocated in main; only characters and their skills are heap-allocated |
| `GameState *` | `game_state_create` | `game_state_free` | Lives in `AppResources` |
| `UnitSprites` | `unit_sprites_load` | `unit_sprites_unload` | Lives in `AppResources` |
| `StructureSprites` | `structure_sprites_load` | `structure_sprites_unload` | Lives in `AppResources` |
| `Terrain *` array | `terrain_init_all` | `terrain_unload_all` | Stack array in main; textures are unloaded |
| `ActionIcons` | `action_icons_load` | `action_icons_unload` | Lives in `AppResources` |
| `GridConfig *` | `grid_init` | `free()` directly | Lives in `AppResources` |
| `Modal *` | `modal_create` | `modal_free` | Lives inside `GameState` |

## Conventions & Patterns

### Naming
- Functions: `lower_snake_case`
- Types (structs, enums): `TitleCase` or `PascalCase`
- Constants / macros: `UPPER_SNAKE_CASE`
- File-private helpers: `static` functions; no leading underscore required

### Memory Management
- **No global variables or singletons**. All state is passed through parameters or contained in structs. The only exception is the file-static `s_app_resources` pointer in `main.c` used strictly for `atexit` cleanup.
- **Create/Free pairs**: Most modules provide `*_create` / `*_free` or `*_init` / `*_unload`. If you add allocation in `*_create`, add a matching free path.
- **NULL safety**: Use `NULL_CHECK_VOID(ptr)` and `NULL_CHECK_RET(ptr, retval)` macros instead of raw `if (!ptr)` checks. These log to stderr in debug builds.
- **Explicit comparisons**: Prefer `if (ptr == NULL)` over `if (!ptr)`. Prefer `if (count > 0)` over `if (count)`.

### Code Quality & Size Limits
- **File size cap**: No source file should exceed 200 lines. When a file grows beyond this limit, split it by extracting cohesive groups of functions into new modules.
- **Function size cap**: No function body should exceed 100 lines. Extract helper functions (`static` where appropriate) to keep each function readable and testable.
- **Dedicated purpose**: Every source and header file must serve exactly one purpose. Do not create god files that accumulate unrelated types, logic, and data. When a file's responsibility drifts, split it into focused modules (e.g., data declarations vs. execution logic).
- **Single responsibility**: Each function should do one thing and do it orthogonally. If a function mixes logic from different domains (e.g., rendering + game rules), split it.
- **Global mutable state**: Avoid global variables and global mutable state. Pass state explicitly through parameters or keep it inside well-defined structs.
- **Const correctness**: Use `const` wherever a pointer or value is not intended to be mutated (function parameters, read-only data, string literals, lookup tables).

### Module Boundaries
- `render/` may read from `types.h` and `input.h` but must not call `game_logic`, `combat`, or `actions` functions except where rendering requires derived display data.
- `input/` may call `game_logic` and `actions` to execute user commands, but should not embed rule logic (e.g., damage formulas).
- `game/` modules may freely call each other, but prefer keeping map-generation logic out of combat logic.
- `ui/` is a leaf layer: it depends on raylib and `types.h`, not on game rules.

### Build System
- `build/premake5.lua`: Defines workspace, raylib download, platform defines, compiler flags. Do not change without strong reason.
- `BuildAndLaunch.sh`: Convenience script. Safe to modify for local workflow tweaks.
- `.vscode/settings.json`, `tasks.json`, `launch.json`: IDE configuration. Update if include paths change.

### clangd / compile_commands.json
A `compile_commands.json` exists at the repository root. It captures the exact compiler flags (includes, defines, `-std=c17`) for every project source file plus raylib. It was generated with `bear` and `compiledb` so that **agents and editors** can reason about the code with accurate cross-file knowledge.

**What agents should know:**
- `clangd` is installed at `/usr/bin/clangd`.
- Before claiming an edit is correct, run a quick check:
  ```bash
  clangd --compile-commands-dir=. --check=src/game/actor.c
  ```
  If it prints no `error:` lines, the file parses cleanly against the full project context.
- To verify a header parses in the correct module context:
  ```bash
  clangd --compile-commands-dir=. --check=src/types.h
  ```
  (clangd infers the flags from any `.c` file that includes it.)

**When to regenerate:**
- If you add/remove source files, regenerate with:
  ```bash
  make clean && bear -- make
  ```
  If the build is currently broken and `bear` can't capture everything, generate manually (script in `scripts/gen_compile_commands.py` or just extend the JSON).
- If you change include paths, defines, or compiler flags in `build/premake5.lua`, run `cd build && ./premake5 gmake` first, then regenerate.

**VS Code integration:**
- `.vscode/settings.json` points the `clangd` extension at the workspace root.
- This means the user also gets autocomplete, diagnostics, and go-to-definition in their editor while the agent edits files.

## Common Tasks for Agents

### Adding a new unit class
1. Add stats to a new `UnitClass` constant in `src/game/class_data.c`.
2. Add `class_get_*()` accessor in `src/game/class_data.h`.
3. If the class belongs to a new archetype, add the archetype first (see below).
4. Update `character_init_from_class` in `src/game/actor.c` if the class needs special initialization.
5. Add sprite loading in `src/render/unit_sprites.c/h`.
6. Add faction initialization in `src/game/faction_init.c` if the class is faction-locked.

### Adding a new archetype
1. Append the new archetype to the `Archetype` enum in `src/types.h`.
2. Add its slot layout to `ARCHETYPE_CONFIGS[]` in `src/game/archetype.c`.
3. Add its base-class mapping to `archetype_get_base_class()` in `src/game/archetype.c`.
4. Create characters by passing the new `Archetype` value to `character_array_create_from_class()` or `character_create_from_class()`.

### Adding a new skill
1. Add skill copy function in `src/game/skill_data.c/h` (model after `action_copy_spear_strike`).
2. Add icon texture to `ActionIcons` in `src/game/actions.h` and load/unload it.
3. Hook the skill into character initialization in `src/game/actor.c`.
4. If the skill needs new targeting logic, update `input_handle_action_click` in `src/input/input.c`.

### Adding a new terrain type
1. Add entry to `TerrainType` enum in `src/game/terrain.h`.
2. Add texture path and initialization in `src/game/terrain.c`.
3. Add `BiomeConfig` entry in `src/game/biome_config.c` if it should appear in map generation.
4. Update `TERRAIN_COUNT` if needed.

### Adding a new structure
1. Add to `Structure` type in `src/types.h` (if changing fields) or just use existing fields.
2. Add sprite loading in `src/render/structure_sprites.h`.
3. Add placement logic in `src/game/structure_generation.c/h`.
4. Add map interaction in `src/game/map.c` if the structure changes passability or lootability rules.

### Modifying turn flow or victory conditions
1. Edit `src/game/game_logic.c/h`.
2. `game_next_turn` advances faction index and checks for defeat.
3. `game_check_victory_conditions` determines win/loss.
4. If adding a new phase, extend `GamePhase` enum.

### Adding a new UI screen
1. Create a new module under `src/ui/` (`.c/.h` pair).
2. Keep it state-driven: `*_init`, `*_update`, `*_render`, `*_free`.
3. Wire it into the mode switcher in `src/main.c`.

## Safety Rules (Do Not Break)

1. **Do not introduce global variables or singletons**. Pass state explicitly.
2. **Do not call `malloc` without a matching free path** (or add it to `AppResources` cleanup).
3. **Do not modify `src/types.h` struct layouts** without auditing every consumer in `src/game/`, `src/render/`, and `src/input/`.
4. **Do not change `build/premake5.lua`** build rules, platform defines, or raylib linkage without explicit confirmation.
5. **Do not add threads**. The game loop is single-threaded and raylib is not thread-safe for most operations.
6. **Do not embed game rules inside `render/`**. Rendering should be a pure function of game state.
7. **Keep `DEBUG_LOG` checks** when adding null guards; use the existing `NULL_CHECK_*` macros.

## File Checklist for Common Starting Points

| Task | Start File |
|------|-----------|
| Understand game loop & init order | `src/main.c` |
| Understand data model | `src/types.h` |
| Add/modify units or classes | `src/game/actor.c/h`, `src/game/class_data.c/h`, `src/game/archetype.c/h` |
| Add/modify combat rules | `src/game/combat.c/h` |
| Change turn flow / AI / victory | `src/game/game_logic.c/h` |
| Change map generation | `src/game/map.c/h`, `src/game/biome_config.c/h` |
| Change input handling | `src/input/input.c/h` |
| Change drawing | `src/render/rendering.c/h` |
| Change UI chrome (buttons, menus) | `src/ui/button.h`, `src/ui/modal.c/h`, `src/ui/menu.c/h` |
| Change build | `build/premake5.lua` |

## Need More Context?

If something is unclear, ask for a specific file or behavior to inspect rather than guessing. The codebase is small enough that a targeted read of the relevant `*.h` file usually resolves ambiguity.
