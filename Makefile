.PHONY: bd, rd, br, rr, c

c:
	@cmake -E remove_directory build
	@cmake -E remove_directory bin

bwr:
	@cmake -E make_directory build
	@cmake -B build/release -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PLATFORM=WINDOWS
	@cmake --build build/release --parallel 12
	@cmake -E make_directory bin
	@cmake -E copy build/release/compile_commands.json build/compile_commands.json
	@cmake -E copy build/release/app.exe bin/release/app.exe

rwr: bwr
	@./bin/release/app.exe ./roms/launch.ini

bwd:
	@cmake -E make_directory build
	@cmake -B build/debug -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_BUILD_PLATFORM=WINDOWS
	@cmake --build build/debug --parallel 12
	@cmake -E make_directory bin
	@cmake -E copy build/debug/compile_commands.json build/compile_commands.json
	@cmake -E copy build/debug/lib/auxum/auxum_test.exe bin/debug/test/auxum_test.exe
	@cmake -E copy build/debug/lib/cchip8/cchip8_test.exe bin/debug/test/cchip8_test.exe
	@cmake -E copy build/debug/app.pdb bin/debug/app.pdb
	@cmake -E copy build/debug/app.exe bin/debug/app.exe

rwd: bwd
	@./bin/debug/app.exe ./roms/launch.ini

t:
	@./bin/debug/test/auxum_test.exe
	@./bin/debug/test/cchip8_test.exe