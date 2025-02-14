.PHONY: bd, rd, br, rr, c

c:
	@cmake -E remove_directory build
	@cmake -E remove_directory bin

br:
	@cmake -E make_directory build
	@cmake -B build/release -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja -DCMAKE_BUILD_TYPE=Release
	@cmake --build build/release --parallel 12
	@cmake -E make_directory bin
	@cmake -E copy build/release/compile_commands.json build/compile_commands.json
	@cmake -E copy build/release/EBox.exe bin/release/EBox.exe

rr: br
	@./bin/release/EBox.exe

bd:
	@cmake -E make_directory build
	@cmake -B build/debug -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja -DCMAKE_BUILD_TYPE=Debug
	@cmake --build build/debug --parallel 12
	@cmake -E make_directory bin
	@cmake -E copy build/debug/compile_commands.json build/compile_commands.json
	@cmake -E copy build/debug/lib/auxum/auxum_test.exe bin/debug/test/auxum_test.exe
	@cmake -E copy build/debug/lib/cchip8/cchip8_test.exe bin/debug/test/cchip8_test.exe
	@cmake -E copy build/debug/EBox.pdb bin/debug/EBox.pdb
	@cmake -E copy build/debug/EBox.exe bin/debug/EBox.exe

rd: bd
	@./bin/debug/EBox.exe

t:
	@./bin/debug/test/auxum_test.exe
	@./bin/debug/test/cchip8_test.exe