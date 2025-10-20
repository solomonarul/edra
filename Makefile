# This Makefile is meant to be used only in development environments.

.DEFAULT_GOAL := _bruh

TARGET = edra

_bruh:
	@echo "You didn't read the README, did you?"

c:
	@cmake -E remove_directory build && echo "[INFO]: Removed build directory."
	@cmake -E remove_directory .cache && echo "[INFO]: Removed .cache directory."
	@cmake -E remove_directory out && echo "[INFO]: Removed out directory."

bud:
	@cmake -B build -S . --preset unix-debug
	@cmake --build build -j${nproc}
	@cmake --install build --prefix out

bur:
	@cmake -B build -S . --preset unix-release
	@cmake --build build -j${nproc}
	@cmake --install build --prefix out

but:
	@cmake -B build -S . --preset unix-test
	@cmake --build build -j${nproc}
	@cmake --install build --prefix out

bub:
	@cmake -B build -S . --preset unix-benchmarks
	@cmake --build build -j${nproc}
	@cmake --install build --prefix out

bwd:
	@cmake -B build -S . --preset windows-debug
	@cmake --build build
	@cmake --install build --prefix out

bwr:
	@cmake -B build -S . --preset windows-release
	@cmake --build build
	@cmake --install build --prefix out

bwt:
	@cmake -B build -S . --preset windows-test
	@cmake --build build
	@cmake --install build --prefix out

bwb:
	@cmake -B build -S . --preset windows-benchmarks
	@cmake --build build
	@cmake --install build --prefix out

rut:
	@(valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/auxum_test > bin/output_auxum_test.log 2>&1) || echo "[EROR]: auxum_test failed! Check bin/output_auxum_test.log!"
	@(valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/cbf_test > bin/output_cbf_test.log 2>&1) || echo "[EROR]: cbf_test failed! Check bin/output_cbf_test.log!"
	@(valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/cchip8_test > bin/output_cchip8_test.log 2>&1) || echo "[EROR]: cchip8_test failed! Check bin/output_cchip8_test.log!"
	@rm -rf vgcore.*
	@echo "[INFO]: Tests ran successfully!"

urgency:
	@(valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/cbf_bench >> bin/output_cbf_bench.log 2>&1)

rb:	
	@./out/bin/cbf_bench

r:
	@./out/bin/${TARGET}

v:
	@rm -f output.log
	@valgrind --suppressions=./.dev/valgrind.supp --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/edra >> output.log 2>&1