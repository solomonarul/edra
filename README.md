# Edra

## How to compile:
- specify SDL3_DIR in the build cache of CMake if needed.
- run make b{platform}{d/r} for an automated build.
    - platforms:
        - w - Windows
        - u - Unix
    - d / r -> debug / release.
- make r for an automated run.

### Running details:
- read [this](./roms/README.md).

## Part of this project:
- [auxum](https://github.com/solomonarul/auxum), an utility library.
- [cchip8](https://github.com/solomonarul/cchip8), agnostic CHIP8 module.

## TODO / nice to haves:
- [x] Single threaded variant.
- [ ] Generalized error handling.
- [x] INI parsing for configs.
- [ ] 1.0 release on Github repo.
- [ ] CHIP8 variants / "quirks".
- [ ] Multiple frontends other than SDL (vanilla CHIP8 fits nicely in the terminal) (SDL_Thread will be kept for threads, I love SDL_Thread).
- [ ] Try as much as possible to avoid the heap and keep data inside the CHIP8 struct.
- [ ] Custom font support?
- [ ] Fix multithreaded and -1 speed combo resulting in 60ips (I don't think there is any fix to that?).
- [ ] Reduce overhead to get more MIPS.
- [ ] CHIP8 Database support?? (JSON: https://github.com/chip-8/chip-8-database, maybe convert to our ini format?)

---

<a href="https://brainmade.org/">
    Made with <3 by a human.
    <img src="https://brainmade.org/88x31-light.png" align="right">
</a>