# Chip-8

This Chip-8 implementation is very simple and is just a test to get started with 
development of emulators in general.

### Installation

Download the project or use git to clone it:
```shell
$ git clone https://github.com/Excse/Chip-8.git
```

Navigate to the directory where the project is located and execute these commands:
```shell
$ mkdir build && cd build
$ cmake ..
$ make
```

To start a rom you now just need to enter this command:
```shell
$ ./chip8_emulator <rom location>
```
e.g.:
```shell
$ ./chip8_emulator "../resources/roms/games/Pong (1 player).ch8"
```
