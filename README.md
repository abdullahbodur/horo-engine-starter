# Horo Engine Starter

`horo-engine-starter` is a minimal template project that shows how to integrate
`horo-engine` as a git submodule and ship a small runnable game sample.

This repository is intentionally simple so new users can copy it, rename it,
and start building their own game quickly.

## What this template provides

- A small C++ app under `app/` linked against `HoroEngine`
- A sample scene under `assets/scenes/`
- A placeholder/fake asset under `assets/models/`
- Cross-platform CMake presets (Linux/macOS + Windows)
- GitHub Actions CI for multi-OS build validation
- An additional engine integration test workflow (`engine-test.yml`)

## Repository layout

- `app/` — starter game application
- `assets/` — sample scene and placeholder assets
- `engine/` — `horo-engine` submodule
- `.github/workflows/` — CI workflows

## Prerequisites

- CMake `3.25+`
- A C++20 compiler
  - Linux: GCC/Clang
  - Windows: Visual Studio 2022
  - macOS: Apple Clang
- Ninja (for non-MSVC presets)

## Getting started

### 1) Clone and initialize submodules

```bash
git clone https://github.com/abdullahbodur/horo-engine-starter.git
cd horo-engine-starter
git submodule update --init --recursive
```

### 2) Configure and build

Linux/macOS:

```bash
cmake --preset debug
cmake --build --preset debug
```

Windows (MSVC):

```bash
cmake --preset debug-msvc
cmake --build build/debug-msvc --config Debug
```

Or simply:

```bash
make build            # Linux/macOS (debug)
make build CONFIG=debug-msvc  # Windows MSVC
```

### 3) Run

```bash
make run-editor       # start in editor mode
make run-play         # start in play mode
```

The `Makefile` automatically detects whether the `engine/` submodule is present:

| Situation | Behaviour |
|-----------|-----------|
| `engine/` submodule present | builds from source, then launches `./build/<config>/bin/HoroStarterApp` |
| `engine/` submodule removed | delegates to the installed HoroEngine IDE binary via `HORO_ENGINE_BINARY` |

**Using the installed IDE binary (no submodule):**

```bash
# Remove the engine submodule when you want IDE-only workflow:
git rm engine
# Then just run:
make run-editor
# Or point to a specific binary:
HORO_ENGINE_BINARY=/path/to/HoroEngine make run-editor
```

The IDE binary will open this project directory directly.

## Using this as your own game template

1. Create a new repository from this template.
2. Keep `engine/` as a submodule (or point it to your own fork).
3. Replace sample assets with your own.
4. Extend `app/main.cpp` with your game logic.
5. Keep CI enabled to validate all target operating systems.
