.PHONY: bwd, rwd, bwr, rwr, c

c:
	@cmake -E remove_directory build
	@cmake -E remove_directory bin

bur:
	@cmake -E make_directory build
	@cmake -B build/release -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PLATFORM=UNIX
	@cmake --build build/release --parallel 12
	@cmake -E make_directory bin
	@cmake -E copy build/release/compile_commands.json build/compile_commands.json
	@cmake -E copy build/release/edra bin/release/edra

rur: bur
	@./build/release/edra ./roms/launch.ini

bwr:
	@cmake -E make_directory build
	@cmake -B build/release -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PLATFORM=WINDOWS
	@cmake --build build/release --parallel 12
	@cmake -E make_directory bin
	@cmake -E copy build/release/compile_commands.json build/compile_commands.json
	@cmake -E copy build/release/edra.exe bin/release/edra.exe

rwr: bwr
	@./bin/release/edra.exe roms/launch.ini

bud:
	@cmake -E make_directory build
	@cmake -B build/debug -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_BUILD_PLATFORM=UNIX
	@cmake --build build/debug --parallel 12
	@cmake -E make_directory bin
	@cmake -E copy build/debug/compile_commands.json build/compile_commands.json
	@cmake -E copy build/debug/lib/auxum/auxum_test bin/debug/test/auxum_test
	@cmake -E copy build/debug/lib/cchip8/cchip8_test bin/debug/test/cchip8_test
	@cmake -E copy build/debug/edra bin/debug/edra

rud: bud
	@./bin/debug/edra roms/launch.ini

bwd:
	@cmake -E make_directory build
	@cmake -B build/debug -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_BUILD_PLATFORM=WINDOWS
	@cmake --build build/debug --parallel 12
	@cmake -E make_directory bin
	@cmake -E copy build/debug/compile_commands.json build/compile_commands.json
	@cmake -E copy build/debug/lib/auxum/auxum_test.exe bin/debug/test/auxum_test.exe
	@cmake -E copy build/debug/lib/cchip8/cchip8_test.exe bin/debug/test/cchip8_test.exe
	@cmake -E copy build/debug/edra.pdb bin/debug/edra.pdb
	@cmake -E copy build/debug/edra.exe bin/debug/edra.exe

rwd: bwd
	@./bin/debug/edra.exe roms/launch.ini

tw:
	@./bin/debug/test/auxum_test.exe
	@./bin/debug/test/cchip8_test.exe