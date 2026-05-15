# AI Agent Instructions

> See `../AGENTS.md` in the repository root for the **canonical** project guide (architecture, conventions, module map, and safety rules).

Quick summary
- Language: C (C17), small single-binary game using raylib.
- Build: premake -> GNU make. Binary lives in `bin/<Config>/Grand-Grid-Dynasty` (debug default: `bin/Debug/Grand-Grid-Dynasty`).
- Canonical docs: `AGENTS.md` (root), `src/CODEMAP.md` (module graph).

How to build & run
- Regenerate makefiles: `cd build && ./premake5 gmake` (or `./premake5.osx gmake` on macOS).
- Build: run `make` from repository root. Use `make config=release_x64` for release.
- Quick launch script: `./BuildAndLaunch.sh linux` will regenerate premake, build and run the Debug binary.
- The build fetches and compiles `raylib` into `external/raylib-master` when needed (see `build/premake5.lua`).
