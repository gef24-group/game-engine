# Development

## Editor
Microsoft Visual Studio Code is the recommended code editor for this project. Please install the extensions listed in [`extensions.json`](.vscode/extensions.json)  
`clangd` is the recommended language server. The settings in [`settings.json`](.vscode/settings.json) configure it for code completion, diagnostics, go-to-definition, and formatting

## Debugging
A debug launch configuration is provided in [`launch.json`](.vscode/launch.json). This, along with the __CMake Tools__ extension, allows you to debug the engine.  
For example, to debug a game running as a client in the client-server mode

1. Click the __CMake__ icon in the activity bar
2. In the __Project Status__ group under the CMake side bar, click __Delete Cache and Reconfigure__
3. In the __Project Outline__ group under the CMake side bar, right-click the game you want to debug, and select __Set as Launch/Debug Target__
4. Edit the `args` property in [`launch.json`](.vscode/launch.json) to configure the client in the client-server mode
```json
"args": ["--mode", "cs", "--role", "client"]
```
5. Click the __Run and Debug__ icon in the activity bar
6. In the __Run and Debug__ side bar, select the __Debug Game__ configuration
7. Click the green play button to start debugging
8. This will build the game in the debug configuration and launch a debugger
9. The client will wait until it can establish a connection with a server, so start the game server in another shell
10. You can now set breakpoints and debug the engine

### Platform-Specific Notes
- For __GCC__ based platforms, set `MIMode` in [`launch.json`](.vscode/launch.json) to `gdb`
- On Windows with __MSVC__, set `type` in [`launch.json`](.vscode/launch.json) to `cppvsdbg`

## Sanitized builds
Memory leaks and thread race conditions can be detected by configuring the build with appropriate sanitizers  
This is supported only on __Clang__ and __GCC__ compilers, and can be enabled by running

```bash
make configure SANITIZER=<thread | address | ...>
```
Set the `SANITIZER` variable to `thread`, `address`, or another supported sanitizer  
Running the sanitized game build will display detected errors in the shell

## Profiling
To build a profiled version that streams [Tracy](https://github.com/wolfpld/tracy) profiling data, configure the project with `PROFILE=ON`

```bash
make configure PROFILE=ON            # Ubuntu or macOS
.\scripts\configure.ps1 -PROFILE ON  # Windows 11
```

By default, profiling data is streamed over port `9000`. If running multiple network endpoints or games on the same machine, specify a different port when launching a new process. Use the `TRACY_PORT` argument to set a custom port.

```bash
make play GAME=<game> ARGS="<game_args>" TRACY_PORT=<port>                   # Ubuntu or macOS
.\scripts\play.ps1 -GAME <game> -GAME_ARGS "<game_args>" -TRACY_PORT <port>  # Windows 11
```