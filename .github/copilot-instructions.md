# Copilot / AI agent instructions for EinallRivals

This file contains focused, actionable guidance to help an AI coding agent be productive in this repository.

Quick summary
- Language: C (C17), small single-binary game using raylib.
- Build: premake -> GNU make. Binary lives in `bin/<Config>/EinallRivals` (debug default: `bin/Debug/EinallRivals`).

How to build & run (developer workflows)
- Regenerate makefiles: `cd build && ./premake5 gmake` (or `./premake5.osx gmake` on macOS).
- Build: run `make` from repository root. Use `make config=release_x64` for release.
- Quick launch script: `./BuildAndLaunch.sh linux` will regenerate premake, build and run the Debug binary.
- The build fetches and compiles `raylib` into `external/raylib-master` when needed (see `build/premake5.lua`).

Project architecture (big picture)
- Single executable game structured as C modules under `src/` with headers alongside. Key entry: `src/main.c`.
- Rendering and presentation: `src/rendering.c` and `src/rendering.h` wrap calls to raylib; prefer using the `RenderContext` API defined there.
- Game model and logic: `src/game_logic.c`, `src/actor.c`, `src/map.c`, `src/terrain.c` implement state and rules. `GameState` and `TroopGroup` are central data flows.
- Input and UI: `src/input.c` handles mouse clicks and selection; UI draw code lives in `src/rendering.c` (e.g., `render_game_full`).
- Resource loading: `unit_sprites.c`, `terrain.c` and `biome_config.c` manage sprite/terrain assets and configuration.

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
- Keep changes minimal and consistent with existing style (C17, no new global state unless necessary).
- Preserve ownership/cleanup semantics (if you allocate memory in `*_create` add matching `*_free` or update calling sites).
- Prefer adding small helper functions in the same module rather than moving large code blocks across files.
- When changing render code, prefer adding a new `render_*` helper and call it from `render_game_full` to avoid regressions.

What not to change without confirmation
- The premake build rules and `Makefile` conventions — altering them can break cross-platform builds.
- Core data layout types in `src/types.h` (changing structs will cascade widely).

If something is unclear or you need more context, ask for a specific file or behavior to inspect.

---
Please review and tell me which parts need more detail or examples.
