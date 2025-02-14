# This is a work in progress and will probably be split across multiple repos.

## How to compile:
- use make bd or make br to generate the build cache.
- specify SDL3_DIR in the build cache generated after the failure (if needed).
- do it again.
- make rd and make rr to run the results.

### We currently have:
- Auxum, an utility library.
- cchip8, multithreaded CHIP8 emulator.
- app, which is the GUI, written with SDL3.