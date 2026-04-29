CONFIG             ?= debug
HORO_ENGINE_BINARY ?= HoroEngine
ENGINE_SCRIPT      := engine/scripts/dev.py

.PHONY: all build run-editor run-play release clean

# ── Build ────────────────────────────────────────────────────────────────────

ifeq ($(wildcard $(ENGINE_SCRIPT)),)

# IDE mode: engine submodule not present — delegate to installed IDE binary.
build:
	@echo "Engine submodule not found. Use the HoroEngine IDE to open this project."
	@exit 1

run-editor:
	$(HORO_ENGINE_BINARY) --editor --project .

run-play:
	$(HORO_ENGINE_BINARY) --play --project .

else

# Submodule mode: engine source is present — build and run locally.
all: build

build:
	cmake --preset $(CONFIG)
	cmake --build --preset $(CONFIG)

run-editor: build
	python3 $(ENGINE_SCRIPT) run-editor --config $(CONFIG)

run-play: build
	python3 $(ENGINE_SCRIPT) run-play --config $(CONFIG)

endif

# ── Shared ───────────────────────────────────────────────────────────────────

release:
	$(MAKE) CONFIG=release build

clean:
	rm -rf build
