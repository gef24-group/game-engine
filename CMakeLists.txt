cmake_minimum_required(VERSION 3.10)

# Set project name
project(game-engine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add CMake modules to find external libraries
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find SDL2 and SDL2_image packages
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

# Include SDL2, SDL2_image, and engine header files
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/engine/includes
)

# Recursively find all source files in the engine directory
file(GLOB_RECURSE ENGINE_SRC "src/engine/*.cpp")

# Get all game directories
file(GLOB GAMES "src/games/*")

foreach(GAME_DIR ${GAMES})
    # Get the name of the game (the folder name)
    get_filename_component(GAME_NAME ${GAME_DIR} NAME)

    # Recursively find game source files
    file(GLOB_RECURSE GAME_SRC ${GAME_DIR}/*.cpp)

    # Find game asset files
    file(GLOB GAME_ASSETS ${GAME_DIR}/assets/*)

    # Set the output directory for the executable
    set(GAME_OUTPUT_DIR ${CMAKE_BINARY_DIR}/${GAME_NAME})

    # Create an executable for each game
    add_executable(${GAME_NAME} ${GAME_SRC} ${ENGINE_SRC})

    # Set the output directory for the executable
    set_target_properties(${GAME_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${GAME_OUTPUT_DIR}
    )

    # Link SDL2 and SDL2_image libraries to the game
    target_link_libraries(${GAME_NAME} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})

    foreach(ASSET ${GAME_ASSETS})
        # Ensure the output assets directory exists
        add_custom_command(TARGET ${GAME_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${GAME_OUTPUT_DIR}/assets"
        )

        # Copy asset files to the output assets directory
        add_custom_command(TARGET ${GAME_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${ASSET} "${GAME_OUTPUT_DIR}/assets"
        )
    endforeach()
endforeach()
