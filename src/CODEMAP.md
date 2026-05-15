# Source Code Map

This file is a quick-reference graph of the `src/` directory for agents navigating the codebase.

## Module Dependency Graph

```
                    +------------------+
                    |   src/main.c     |  <-- Entry point, mode switcher
                    +--------+---------+
                             |
        +--------------------+--------------------+
        |                    |                    |
   +----v----+         +----v----+         +----v----+
   |  Menu   |         | Input   |         | Render  |
   | (ui/)   |         | (input/)|         | (render/)|
   +----+----+         +----+----+         +----+----+
        |                   |                    |
        +-------------------+--------------------+
                            |
                     +------v-------+
                     | Game Logic   |
                     | (game/)      |
                     +------+-------+
                            |
              +-------------+-------------+
              |             |             |
         +----v----+   +----v----+   +----v----+
         |  Map    |   | Combat  |   |  Actor  |
         | (map/)  |   | (combat)|   | (actor) |
         +----+----+   +----+----+   +----+----+
              |             |             |
              +-------------+-------------+
                            |
                     +------v-------+
                     |    types.h   |  <-- Shared structs (leaf layer)
                     +--------------+
                            |
                     +------v-------+
                     |   core/      |  <-- Utilities (grid math)
                     +--------------+
```

## Legend

- **Upward arrows** = depends on / includes headers from
- **Downward arrows** = is consumed by / called from
- `types.h` is included by nearly every module; omitted from most local edges for clarity.

## Directory-to-Directory Edges

| From | To | Why |
|------|-----|-----|
| `main.c` | all | Orchestrates initialization, cleanup, and the frame loop. |
| `input/` | `game/` | Executes skills (`actions.h`), combat (`combat.h`). |
| `input/` | `ui/` | Reads `button.h` for hit-testing. |
| `render/` | `game/` | Reads `map.h`, `combat.h` for debug/drawing data. |
| `render/` | `ui/` | Reads `button.h`, `modal.h`, `menu.h` to draw chrome. |
| `ui/` | `game/` | `modal.h` uses `Character*` for level-up screens. |
| `game/` | `core/` | `utils.h` for grid allocation and cell flag utilities. |

## Key Type Owners

| Type | Defined in | Owned by | Freed by |
|------|-----------|----------|----------|
| `Point *mapArr` | `types.h` | `main.c` (AppResources) | `map_free` |
| `Character *` arrays | `types.h` | `Faction` structs | `character_array_free` via `factions_free_actors` |
| `GameState *` | `game_logic.h` | `main.c` (AppResources) | `game_state_free` |
| `Skill` arrays (per character) | `types.h` | `Character` | `skill_free` inside `character_array_free` |
| `GridConfig *` | `types.h` | `main.c` (AppResources) | `free()` directly |
| `Modal *` | `modal.h` | `GameState` | `modal_free` inside cleanup |
| `Texture2D` (raylib) | `raylib.h` | Various loaders | `UnloadTexture` / `unit_sprites_unload` / `terrain_unload_all` |

## Where Logic Lives (Feature -> File Mapping)

| Feature | Primary File | Secondary Files |
|---------|-------------|-----------------|
| Title screen | `ui/menu.c` | `main.c` (mode switcher) |
| Pause modal | `ui/modal.c` | `main.c` (ESC handler) |
| Unit selection | `input/input.c` | `render/rendering.c` (highlight) |
| Movement | `input/input.c` | `game/map.c` (range calc) |
| Combat attack | `game/combat.c` | `input/input.c` (trigger) |
| Combat forecast | `game/combat.c` | `render/rendering.c` (display) |
| Turn order | `game/game_logic.c` | `main.c` (loop) |
| AI turns | `game/game_logic.c` | `main.c` (branch) |
| Victory/defeat | `game/game_logic.c` | `main.c` (render overlay) |
| Map generation | `game/map.c` | `game/biome_config.c`, `game/terrain.c` |
| Structure placement | `game/structure_generation.c` | `game/map.c` |
| Spawning | `game/spawning.c` | `main.c` (caller) |
| Level-up / promotion | `game/actor.c` | `ui/modal.c` (class choice UI) |

## Adding a New Module

If you create a new subdirectory under `src/`:

1. Add the directory to `build/premake5.lua` in the `files` directive if it does not already match the glob `../src/*/*.c`.
2. Include `types.h` in the new module's header.
3. Add the header to the module's `.c` file; do not add it to unrelated modules.
4. If the new module is a leaf (e.g., a new UI primitive), keep it dependency-free of `game/`.
5. If the new module is a game system, place initialization in `main.c` and add resources to `AppResources` / `cleanup.h`.
