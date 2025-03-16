# This Makefile is meant to be used only in development environments.

.DEFAULT_GOAL := _bruh

TARGET = edra

_bruh:
	@echo "You didn't read the README, did you?"

c:
	@cmake -E remove_directory build && echo "[INFO]: Removed build directory."
	@cmake -E remove_directory .cache && echo "[INFO]: Removed .cache directory."
	@cmake -E remove_directory bin && echo "[INFO]: Removed .cache directory."

bud:
	@cmake -B build -S . --preset unix-debug
	@cmake --build build -j${nproc}
	@cmake -E copy_directory assets ./bin/assets

bur:
	@cmake -B build -S . --preset unix-release
	@cmake --build build -j${nproc}
	@cmake -E copy_directory assets ./bin/assets

bwd:
	@cmake -B build -S . --preset windows-debug
	@cmake --build build
	@cmake -E copy_directory assets ./bin/assets

bwr:
	@cmake -B build -S . --preset windows-release
	@cmake --build build
	@cmake -E copy_directory assets ./bin/assets

bvd:
	@cmake -B build -S . --preset vita-debug
	@cmake --build build -j${nproc}
	@mkdir -p bin
	@cp build/edra.vpk bin/edra.vpk

bvr:
	@cmake -B build -S . --preset vita-release
	@cmake --build build -j${nproc}
	@mkdir -p bin
	@cp build/edra.vpk bin/edra.vpk

r:
	@./bin/${TARGET} ./roms/launch.ini