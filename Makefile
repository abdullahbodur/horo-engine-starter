CONFIG ?= debug

.PHONY: all run release clean

all:
	cmake --preset $(CONFIG)
	cmake --build --preset $(CONFIG)

run: all
	./build/$(CONFIG)/bin/HoroStarterApp

release:
	$(MAKE) CONFIG=release all

clean:
	rm -rf build
