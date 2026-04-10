# Horo Engine Starter

`horo-engine-starter` is a minimal template project that shows how to integrate
`horo-engine` as a git submodule and ship a small runnable game sample.

This repository is intentionally simple so new users can copy it, rename it,
and start building their own game quickly.

## What this template provides

- A small C++ app under `app/` linked against `MonolithEngine`
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

### 3) Run

Linux/macOS:

```bash
./build/debug/bin/HoroStarterApp
```

Windows:

```powershell
.\build\debug-msvc\bin\Debug\HoroStarterApp.exe
```

## Using this as your own game template

1. Create a new repository from this template.
2. Keep `engine/` as a submodule (or point it to your own fork).
3. Replace sample assets with your own.
4. Extend `app/main.cpp` with your game logic.
5. Keep CI enabled to validate all target operating systems.
