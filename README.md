# Horo Engine Starter

Open-source example game repo for `horo-engine`.

Amaç:
- `horo-engine`'i `engine/` altında submodule olarak tüketmek
- monolith'e benzeyen ama daha küçük bir sample proje sunmak
- reusable GitHub Actions workflow ile engine PR branch'lerini bu repo içinde doğrulamak

## Repository Layout

- `app/` — küçük sample game app
- `assets/` — sample scene + fake placeholder asset
- `engine/` — `horo-engine` submodule
- `.github/workflows/` — local CI + reusable integration workflow

## Build

```bash
git submodule update --init --recursive
cmake --preset debug
cmake --build --preset debug
```

Windows:

```bash
cmake --preset debug-msvc
cmake --build build/debug-msvc --config Debug
```

## Run

```bash
./build/debug/bin/HoroStarterApp
```

## Reusable workflow mantığı

Bu repo içindeki `reusable-engine-integration.yml` workflow'u, `horo-engine` tarafında açılan PR branch/sha bilgisini input olarak alır:

1. Bu starter repo'yu checkout eder
2. `engine/` klasörüne çağıran branch/sha'yı checkout eder
3. Linux / Windows / macOS üzerinde configure + build dener

Örnek kullanım (`horo-engine` repo'sunda):

```yaml
jobs:
  starter-integration:
    uses: abdullahbodur/horo-engine-starter/.github/workflows/reusable-engine-integration.yml@main
    with:
      engine_repository: ${{ github.repository }}
      engine_ref: ${{ github.event.pull_request.head.sha || github.sha }}
    secrets: inherit
```
