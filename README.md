# This is a work in progress and will probably be split across multiple repos.

## How to compile:
- specify SDL3_DIR in the build cache of CMake.
- make bwd / make bwr to (build) the results.
- make rwd / make rwr to (run + build) the results.

## How to run:
- read [this](./roms/README.md).

### We currently have:
- Auxum, an utility library.
- cchip8, multithreaded CHIP8 emulator.
- app, which is the GUI, written with SDL3.

## TODO / nice to haves:
- [ ] Generalized error handling.
- [ ] INI parsing for configs.
- [ ] 1.0 release on Github repo.
- [ ] CHIP8 variants / "quirks".
- [ ] Single threaded variant.
- [ ] Multiple frontends other than SDL (vanilla CHIP8 fits nicely in the terminal) (SDL_Thread will be kept for threads, I love SDL_Thread).
- [ ] Try as much as possible to avoid the heap and keep data inside the CHIP8 struct.