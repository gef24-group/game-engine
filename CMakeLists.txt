cmake_minimum_required(VERSION 3.11)

# Set project name
project(game-engine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Some libraries are downloaded using FetchContent
include(FetchContent)

# Download the "nlohmann/json" library
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/vendor/json
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# Add SANITIZER support if it is provided
if(SANITIZER)
    set(SANITIZER_FLAGS "-fsanitize=${SANITIZER}")

    # Apply the sanitizer flags on supported compilers
    if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(STATUS "Adding sanitizer flags: ${SANITIZER_FLAGS}")
        add_compile_options(${SANITIZER_FLAGS} -g -O1 -fno-omit-frame-pointer)
        add_link_options(${SANITIZER_FLAGS})
    endif()
endif()

# Add the tracy profiler if PROFILE is ON
if (PROFILE STREQUAL "ON")
    FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/vendor/tracy
    )
    FetchContent_MakeAvailable(tracy)

    add_definitions(-DPROFILE)
endif()

# Add CMake modules to find external libraries
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find required packages
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(Threads REQUIRED)
find_package(cppzmq REQUIRED)

# Include required header files
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/engine/includes
)

# Recursively find all source files in the engine directory
file(GLOB_RECURSE ENGINE_SRC "src/engine/*.cpp")

# Read the .targetgames file
file(READ "${CMAKE_SOURCE_DIR}/.targetgames" TARGET_GAMES)

# Convert TARGET_GAMES to a list (split by lines)
string(REGEX REPLACE "\n" ";" TARGET_GAMES_LIST "${TARGET_GAMES}")

# Initialize an empty list to store the valid games
set(GAMES)

# Directly append all the games from the .targetgames list
foreach(GAME_NAME IN LISTS TARGET_GAMES_LIST)
    list(APPEND GAMES "src/games/${GAME_NAME}")
endforeach()

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

    # Link required libraries to the game
    target_link_libraries(
        ${GAME_NAME}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        Threads::Threads
        cppzmq
        nlohmann_json::nlohmann_json
    )

    if (PROFILE STREQUAL "ON")
        target_link_libraries(${GAME_NAME} TracyClient)
    endif()

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
