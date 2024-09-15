# game-engine
This repo houses a game engine developed for CSC 581 (Game Engine Foundations), Group 15, Fall 2024 @ NC State

## Build
### Prerequisites
To build the game engine and included games, please ensure that you have all prerequisites installed.

#### Ubuntu 24.04
```bash
sudo apt update
sudo apt install build-essential x11-apps libsdl2-2.0-0 libsdl2-dev libsdl2-image-2.0-0 libsdl2-image-dev libzmq3-dev cmake
```

#### macOS Sonoma 14.6.1
Install Homebrew from [brew.sh](https://brew.sh/)
```bash
xcode-select --install
brew install sdl2 sdl2_image zeromq cppzmq cmake
```

### Build all games
Every folder in `src/games` maps to a game.  
To build all games, run:
```bash
make
```

### Build a specific game
To build a specific game, pass the `GAME` variable to `make`.  
For example, if these are the folders inside `src/games`:
```bash
src/games
├── hw1_joshua
├── hw1_mitesh
└── hw1_rohan
```
And you would like to only build `hw1_joshua`, then you would need to run:
```bash
make GAME=hw1_joshua
```

## Play
After a game has been built, you may play it using this command:
```bash
make play GAME=<game>
```
Where `<game>` is one of the games from the `src/games` folder.  
For example, if you would like to play `hw1_rohan` from `src/games`, then you would need to run:
```bash
make play GAME=hw1_rohan
```

## Cleanup
If you would like to clear all build artifacts, please run:
```bash
make clean
```

## Display scaling
To switch between constant and proportional scaling, please use the `x` key.

## References
Please find all references in [References.md](References.md)