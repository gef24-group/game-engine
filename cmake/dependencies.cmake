include(cmake/CPM.cmake)

CPMAddPackage(
    NAME SDL
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.8/SDL2-2.30.8.tar.gz
    URL_HASH SHA256=380c295ea76b9bd72d90075793971c8bcb232ba0a69a9b14da4ae8f603350058
    OPTIONS "SDL_TEST OFF" "SDL_SHARED ON" "SDL_STATIC OFF"
    EXCLUDE_FROM_ALL YES
    SYSTEM YES
)

CPMAddPackage(
    NAME SDL_image
    URL https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-2.8.2.tar.gz
    URL_HASH SHA256=8f486bbfbcf8464dd58c9e5d93394ab0255ce68b51c5a966a918244820a76ddc
    OPTIONS "SDL2IMAGE_INSTALL OFF" "BUILD_SHARED_LIBS ON"
    EXCLUDE_FROM_ALL YES
    SYSTEM YES
)

CPMAddPackage(
    NAME libzmq
    URL https://github.com/zeromq/libzmq/releases/download/v4.3.5/zeromq-4.3.5.tar.gz
    URL_HASH SHA256=6653ef5910f17954861fe72332e68b03ca6e4d9c7160eb3a8de5a5a913bfab43
    OPTIONS "WITH_DOCS OFF" "BUILD_TESTS OFF" "ENABLE_CPACK OFF" "BUILD_SHARED ON" "BUILD_STATIC OFF"
    EXCLUDE_FROM_ALL YES
    SYSTEM YES
)

CPMAddPackage(
    NAME cppzmq
    URL https://github.com/zeromq/cppzmq/archive/refs/tags/v4.10.0.tar.gz
    URL_HASH SHA256=c81c81bba8a7644c84932225f018b5088743a22999c6d82a2b5f5cd1e6942b74
    OPTIONS "CPPZMQ_BUILD_TESTS OFF"
    EXCLUDE_FROM_ALL YES
    SYSTEM YES
)

CPMAddPackage(
    NAME json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
    OPTIONS "JSON_BuildTests OFF"
    EXCLUDE_FROM_ALL YES
    SYSTEM YES
)

CPMAddPackage(
    NAME tracy
    URL https://github.com/wolfpld/tracy/archive/refs/tags/v0.11.1.tar.gz
    URL_HASH SHA256=2c11ca816f2b756be2730f86b0092920419f3dabc7a7173829ffd897d91888a1
    OPTIONS "TRACY_ENABLE ${PROFILE}"
    EXCLUDE_FROM_ALL YES
    SYSTEM YES
)

find_package(Threads REQUIRED)