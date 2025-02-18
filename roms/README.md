# Running

I recommend placing all your ROMs here for better organization.

- Create a launch.ini file.
- Specify the contents in the following format:
```
[system]
module=chip8

[chip8.core]
threaded=<if_use_multiple_threads_here>
speed=<speed_number_here>
input=sdl
output=sdl

[chip8.sdl.colors]
foreground_r=200
foreground_g=200
foreground_b=200
background_r=20
background_g=20
background_b=20

[chip8.sdl.keys]
keys[]=[X, 1, 2, 3, Q, W, E, A, S, D, Z, C, 4, R, F, V]
```
- Launch the executable with the path to launch.ini as the first argument.