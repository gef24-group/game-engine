cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# Set project name
project(game-engine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set RPATH to the same directory as the executable
if(APPLE)
    set(CMAKE_BUILD_RPATH "@executable_path")
elseif(UNIX)
    set(CMAKE_BUILD_RPATH "$ORIGIN")
endif()

# Include configuration options
include(cmake/config.cmake)

# Include dependencies
include(cmake/dependencies.cmake)

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

# Add the PROFILE definition if PROFILE is ON
if (PROFILE STREQUAL "ON")
    add_definitions(-DPROFILE)
endif()

# Include required header files
include_directories(
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
        PRIVATE
        SDL2::SDL2
        SDL2_image::SDL2_image
        cppzmq
        nlohmann_json::nlohmann_json
        Threads::Threads
    )

    if (PROFILE STREQUAL "ON")
        target_link_libraries(${GAME_NAME} PRIVATE TracyClient)
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

    # Copy the game README to the same folder as the executable
    add_custom_command(TARGET ${GAME_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "../${GAME_DIR}/README.md"
            "$<TARGET_FILE_DIR:${GAME_NAME}>"
    )

    # Copy shared libraries to the same folder as the executable
    if (WIN32)
        add_custom_command(
            TARGET ${GAME_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                "$<TARGET_RUNTIME_DLLS:${GAME_NAME}>"
                "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}"
                "$<TARGET_FILE_DIR:${GAME_NAME}>"
            COMMAND_EXPAND_LISTS
        )
    else()
        add_custom_command(
            TARGET ${GAME_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                "$<TARGET_SONAME_FILE:SDL2::SDL2>"
                "$<TARGET_SONAME_FILE:SDL2_image::SDL2_image>"
                "$<TARGET_SONAME_FILE:libzmq>"
                "$<TARGET_FILE_DIR:${GAME_NAME}>"
            VERBATIM
        )
    endif()
endforeach()
