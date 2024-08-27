File Structure: 

Main contains the main update loop and calls other functions to handle engine subsystems.

structs contains the application struct, which includes the SDL Renderer and SDL Window pointers.

init initializes the SDL components

input handles HID Input

draw has the functions to apply the window color, clear the renderer, and render the updated scene.
