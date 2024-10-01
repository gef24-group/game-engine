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
├── hw2_joshua
├── hw2_mitesh
└── hw2_rohan
```
And you would like to only build `hw2_joshua`, then you would need to run:
```bash
make GAME=hw2_joshua
```

<!-- ## Play
After a game has been built, you may play it using this command:
```bash
make play GAME=<game>
```
Where `<game>` is one of the games from the `src/games` folder.  
For example, if you would like to play `hw1_rohan` from `src/games`, then you would need to run:
```bash
make play GAME=hw1_rohan
``` -->

## Playing the game in client-server mode
The following command has to be executed to run the server:
```bash
make play GAME=<game> ARGS="--mode cs --role server"
```

And, The following command has to be executed to run a client:
```bash
make play GAME=<game> ARGS="--mode cs --role client"
```

Where `<game>` is one of the games from the `src/games` folder. *The HW2 games written by Joshua (jjoseph6), Rohan (rjmathe2), 
and Mitesh (magarwa3) lie in the `hw2_joshua`, `hw2_rohan` and `hw2_mitesh` folders respectively.*
For example, if you would like to play `hw2_rohan` from `src/games` in client-server mode, then you would need to run the following 2 commands to run the server and the clients respectively:
```bash
make play GAME=hw2_rohan ARGS="--mode cs --role server"
```
```bash
make play GAME=hw2_rohan ARGS="--mode cs --role client"
```

## Playing the game in peer-to-peer mode
The following command has to be executed to run the listen-server/host:
```bash
make play GAME=<game> ARGS="--mode p2p --role host"
```
And, The following command has to be executed to run a peer:
```bash
make play GAME=<game> ARGS="--mode p2p --role peer"
```
Example: If you would like to play Mitesh's HW2 game in peer-to-peer mode, run the following 2 commands to run the listen-server and the peers respectively:
```bash
make play GAME=hw2_mitesh ARGS="--mode p2p --role host"
```
```bash
make play GAME=hw2_mitesh ARGS="--mode p2p --role peer"
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