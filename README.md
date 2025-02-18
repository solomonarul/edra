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
- [x] Single threaded variant.
- [ ] Generalized error handling.
- [x] INI parsing for configs.
- [ ] 1.0 release on Github repo.
- [ ] CHIP8 variants / "quirks".
- [ ] Multiple frontends other than SDL (vanilla CHIP8 fits nicely in the terminal) (SDL_Thread will be kept for threads, I love SDL_Thread).
- [ ] Try as much as possible to avoid the heap and keep data inside the CHIP8 struct.
- [ ] Custom font support?
- [ ] Fix multithreaded and -1 speed combo resulting in 60ips.
- [ ] Reduce overhead to get more MIPS.
- [ ] CHIP8 Database support?? (JSON: https://github.com/chip-8/chip-8-database, maybe convert to our ini format?)

---

<a href="https://brainmade.org/">
    Made with <3 by a human.
    <img src="https://brainmade.org/88x31-light.png" align="right">
</a>