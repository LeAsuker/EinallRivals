# Copilot / AI agent instructions for Grand-Grid-Dynasty

This file contains focused, actionable guidance to help an AI coding agent be productive in this repository.

Quick summary
- Language: C (C17), small single-binary game using raylib.
- Build: premake -> GNU make. Binary lives in `bin/<Config>/Grand-Grid-Dynasty` (debug default: `bin/Debug/Grand-Grid-Dynasty`).

How to build & run (developer workflows)
- Regenerate makefiles: `cd build && ./premake5 gmake` (or `./premake5.osx gmake` on macOS).
- Build: run `make` from repository root. Use `make config=release_x64` for release.
- Quick launch script: `./BuildAndLaunch.sh linux` will regenerate premake, build and run the Debug binary.
- The build fetches and compiles `raylib` into `external/raylib-master` when needed (see `build/premake5.lua`).

Project architecture (big picture)
- Single executable game structured as C modules organized into subdirectories under `src/`: `core/`, `game/`, `input/`, `render/`, and `ui/`. Key entry: `src/main.c`.
- **Core utilities** (`src/core/`): Common helper functions and utilities (`utils.c/h`).
- **Game logic** (`src/game/`): Core game systems including actors (`actor.c/h`), map generation (`map.c/h`), terrain (`terrain.c/h`), game state (`game_logic.c/h`), combat (`combat.c/h`), structures (`structure.c/h`, `structure_generation.c/h`), and configuration (`biome_config.c/h`, `faction_init.c/h`).
- **Rendering** (`src/render/`): Presentation layer wrapping raylib (`rendering.c/h`). Prefer using the rendering API defined there. Sprite/asset loading (`unit_sprites.c/h`, `structure_sprites.h`).
- **Input** (`src/input/`): User interaction handling (`input.c/h`); processes mouse clicks and selection.
- **UI** (`src/ui/`): User interface components including menus (`menu.c/h`), modals (`modal.c/h`), and buttons (`button.c/h`).

Conventions and patterns to follow (project-specific)
- Ownership: many modules provide `*_create`, `*_init`, `*_free` or `*_unload` pairs. Use existing allocation/free patterns.
- Naming: functions use lower_snake_case and types are TitleCase (e.g., `GameState`, `GridConfig`).
- Actor arrays: dynamic arrays are often allocated and freed manually (see `actor_array_create` / `actor_array_free`). Match that style.
- Rendering split: keep game logic separate from `render_*` functions; rendering reads state but should not mutate game rules.
- Single-threaded main loop: game loop in `main.c` runs on the main thread with `SetTargetFPS(60)`—avoid introducing background threads without care.

Integration points & external deps
- raylib: integrated as a static library built under `external/raylib-master` (premake can download it). API surface used throughout `rendering.c` and `main.c`.
- Platform defines: `build/premake5.lua` sets platform macros (e.g., `PLATFORM_DESKTOP`, `_GLFW_X11`, `_GNU_SOURCE`). Be cautious modifying these flags.

Files to inspect for common tasks (examples)
- Start point & game loop: `src/main.c` — shows initialization order, map generation, faction setup, and cleanup.
- Rendering/UI patterns: `src/rendering.c` — use `render_init`, `render_game_full` for drawing and UI button state.
- Turn / state machine: `src/game_logic.c` — follow `game_state_create`, `game_next_turn`, `game_end_current_turn` for turn flow.
- Build automation & external download: `build/premake5.lua` — how raylib is downloaded/unzipped and buildconfigs are defined.

Editing guidance for AI patches
- **No global variables or singletons**: All state must be passed through function parameters or contained within structs. Do not introduce global state under any circumstance.
- **Memory safety**: When adding new features, check for possible memory leaks. Ensure all allocations have matching deallocations, and verify cleanup paths are called (especially in error cases).
- **Explicit checks**: Be explicit in conditionals and comparisons. Use `if (ptr == NULL)` instead of `if (!ptr)`, and prefer clear comparisons over implicit conversions. Explicit code improves clarity and prevents subtle bugs.
- Keep changes minimal and consistent with existing style (C17).
- Preserve ownership/cleanup semantics: if you allocate memory in `*_create` add matching `*_free` or update calling sites.
- Prefer adding small helper functions in the same module rather than moving large code blocks across files.
- When changing render code, prefer adding a new `render_*` helper and call it from `render_game_full` to avoid regressions.

What not to change without confirmation
- The premake build rules and `Makefile` conventions — altering them can break cross-platform builds.
- Core data layout types in `src/types.h` (changing structs will cascade widely).

If something is unclear or you need more context, ask for a specific file or behavior to inspect.
