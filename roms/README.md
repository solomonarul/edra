# Running

I recommend placing all your ROMs here for better organization.

- Create a launch.ini file.
- Specify the contents in the following format:
```
[system]
module=chip8

[chip8.core]
path=<rom_path>
threaded=true
speed=600
input=sdl
output=sdl

[chip8.input.sdl.keys]
keys[]=[[X], [1], [2], [3], [Q], [W], [E], [A], [S], [D], [Z], [C], [4], [R], [F], [V]]

[chip8.output.sdl.colors]
foreground[]=[200, 200, 200]
background[]=[20, 20, 20]
```
- Make sure that the length of a line is less than 255 characters.
- Launch the executable with the path to launch.ini as the first argument.