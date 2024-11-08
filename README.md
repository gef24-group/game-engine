# game-engine
This repo houses the game engine developed for CSC 581 (Game Engine Foundations), Group 15, Fall 2024 @ NC State

## Build
### Prerequisites
To build the game engine and included games, please ensure that you have all prerequisites installed.

#### Ubuntu 24.04
```bash
sudo apt update
sudo apt install build-essential x11-apps libsdl2-2.0-0 libsdl2-dev libsdl2-image-2.0-0 libsdl2-image-dev libzmq3-dev cmake
```

#### macOS Sequoia 15.0.1
Install Homebrew from [brew.sh](https://brew.sh/)
```bash
xcode-select --install
brew install sdl2 sdl2_image zeromq cppzmq cmake
```

### Build all games
Every folder in `src/games` maps to a game  
The [`.targetgames`](.targetgames) file defines all the games that are supported in the current build  
To build all games defined in [`.targetgames`](.targetgames), run
```bash
make
```

### Build a specific game
To build a specific game, pass the `GAME` variable to `make`  
For example, if these are the games defined in [`.targetgames`](.targetgames)
```bash
hw4_joshua
hw4_rohan
hw4_mitesh
```
And you would like to only build `hw4_joshua`, then you would need to run
```bash
make GAME=hw4_joshua
```

## Play
After a game has been built, you may play it using this command
```bash
make play GAME=<game> ARGS="<args>"
```

## Engine flags
Most games include multi-player support, but require certain flags to be passed to the engine. This is facilitated by the `ARGS` variable  

The flags supported by the engine are
```bash
--mode      [single, cs, p2p]             (default: single)
--role      [server, client, host, peer]  (default: client)
--encoding  [struct, json]                (default: struct)
--server_ip <ip_address>                  (default: localhost)
--host_ip   <ip_address>                  (default: localhost)
--peer_ip   <ip_address>                  (default: localhost)
```

## Examples
### Client-server mode
To run `hw4_mitesh` in the client-server mode, run these commands in different shells  

**Server**
```bash
make play GAME=hw4_mitesh ARGS="--mode cs --role server"
```
**Client**
```bash
make play GAME=hw4_mitesh ARGS="--mode cs --role client"
```

### Peer-to-peer mode
To run `hw4_rohan` in the peer-to-peer mode, run these commands in different shells  

**Host**
```bash
make play GAME=hw4_rohan ARGS="--mode p2p --role host"
```
**Peer**
```bash
make play GAME=hw4_rohan ARGS="--mode p2p --role peer"
```

> The HW4 games written by Joshua (jjoseph6), Rohan (rjmathe2), and Mitesh (magarwa3) lie in the `hw4_joshua`, `hw4_rohan` and `hw4_mitesh` folders respectively

## Cleanup
If you would like to clear all build artifacts, please run
```bash
make clean
```

## Common inputs
Here is a map of inputs common to every game  
| Input        | Action                        |
|--------------|-------------------------------|
| <kbd>p</kbd> | Toggle timeline pause         |
| <kbd>,</kbd> | Slow down the timeline        |
| <kbd>.</kbd> | Speed up the timeline         |
| <kbd>x</kbd> | Toggle display scaling        |
| <kbd>z</kbd> | Toggle hidden zone visibility |

## References
Please find all references in [References.md](References.md)