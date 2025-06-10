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

but:
	@cmake -B build -S . --preset unix-test
	@cmake --build build -j${nproc}
	@cmake -E copy_directory assets ./bin/assets

bub:
	@cmake -B build -S . --preset unix-benchmarks
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

bwt:
	@cmake -B build -S . --preset windows-test
	@cmake --build build
	@cmake -E copy_directory assets ./bin/assets

bwb:
	@cmake -B build -S . --preset windows-benchmarks
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

rut:
	@(valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/auxum_test > bin/output_auxum_test.log 2>&1) || echo "[EROR]: auxum_test failed! Check bin/output_auxum_test.log!"
	@(valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/cbf_test > bin/output_cbf_test.log 2>&1) || echo "[EROR]: cbf_test failed! Check bin/output_cbf_test.log!"
	@(valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/cchip8_test > bin/output_cchip8_test.log 2>&1) || echo "[EROR]: cchip8_test failed! Check bin/output_cchip8_test.log!"
	@rm -rf vgcore.*
	@echo "[INFO]: Tests ran successfully!"

urgency:
	@(valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/cbf_bench >> bin/output_cbf_bench.log 2>&1)

rb:	
	@./bin/cbf_bench

r:
	@./bin/${TARGET}

v:
	@rm -f output.log
	@valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/edra >> output.log 2>&1