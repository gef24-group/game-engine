// This game will not be compiled or analyzed due to breaking API changes
// Please revert to the following commit to compile this game
// 6bd6a670fdbe6e451136c37604f63567b582e833

#include "GameEngine.hpp"
#include "GameObject.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include <string>

Size window_size;
NetworkInfo network_info;

// Game update code
void Update(std::vector<GameObject *> *game_objects) {

    // Get the ball and platform objects
    GameObject *ball = NULL;
    std::vector<GameObject *> platforms;

    for (GameObject *object : *game_objects) {
        if (object->GetName() == "ball") {
            ball = object;
        }
        if (object->GetName().find("platform") == 0) {
            platforms.push_back(object);
        }
    }

    if (ball == NULL || platforms.empty()) {
        return;
    }

    // Update ball's position and handle collisions
    Position ball_pos = ball->GetPosition();
    Velocity ball_vel = ball->GetVelocity();
    Size ball_size = ball->GetSize();

    // Wall boundaries
    GameObject *wall_left = GetObjectByName("wall_left", *game_objects);
    GameObject *wall_right = GetObjectByName("wall_right", *game_objects);
    GameObject *wall_top = GetObjectByName("wall_top", *game_objects);

    wall_left->SetPosition(Position{0, 0});
    wall_left->SetSize(Size{100, window_size.height});

    wall_top->SetPosition(Position{100, 0});
    wall_top->SetSize(Size{window_size.width - 100, 100});

    wall_right->SetPosition(Position{float(window_size.width - 100), 100});
    wall_right->SetSize(Size{100, window_size.height - 100});

    // Collision with left wall
    if (ball_pos.x <= wall_left->GetPosition().x + float(wall_left->GetSize().width)) {
        ball->SetVelocity({-ball_vel.x, ball_vel.y}); // Reflect horizontally
    }

    // Collision with right wall
    if (ball_pos.x + float(ball_size.width) >= wall_right->GetPosition().x) {
        ball->SetVelocity({-ball_vel.x, ball_vel.y}); // Reflect horizontally
    }

    // Collision with top wall
    if (ball_pos.y <= wall_top->GetPosition().y + float(wall_top->GetSize().height)) {
        ball->SetVelocity({ball_vel.x, -ball_vel.y}); // Reflect vertically
    }

    // Collision with platforms
    for (GameObject *platform : platforms) {
        if (ball_pos.y + float(ball_size.height) >= platform->GetPosition().y &&
            ball_pos.x + float(ball_size.width) >= platform->GetPosition().x &&
            ball_pos.x <= platform->GetPosition().x + float(platform->GetSize().width)) {
            // Reflect the ball vertically if a collision is detected
            ball->SetVelocity({ball_vel.x, -ball_vel.y});

            // Optionally, you could break out of the loop after the first collision
            // to avoid multiple collisions being processed at the same time.
            break;
        }
    }

    // If ball goes out of the boundary
    if (ball_pos.y + float(ball_size.height) > 1080) {
        Log(LogLevel::Info, "You lost!");
        app->quit = true;
    }
}

void UpdatePlatform(GameObject *platform, GameObject *wall_left, GameObject *wall_right) {
    bool player_moved = false;
    auto position = platform->GetPosition();
    auto size = platform->GetSize();

    if (app->key_map->key_left.pressed.load()) {
        player_moved = true;
        platform->SetVelocity({platform->GetVelocity().x - 10, platform->GetVelocity().y});
    } else if (app->key_map->key_right.pressed.load()) {
        player_moved = true;
        platform->SetVelocity({platform->GetVelocity().x + 10, platform->GetVelocity().y});
    }

    if (position.x <= wall_left->GetPosition().x + float(wall_left->GetSize().width)) {
        position.x = wall_left->GetPosition().x + float(wall_left->GetSize().width);
    }
    if (position.x + float(size.width) >= wall_right->GetPosition().x) {
        position.x = wall_right->GetPosition().x -
                     float(size.width); // Move platform to the left of the right wall
    }
    platform->SetPosition(position);

    if (!player_moved) {
        platform->SetVelocity({0, platform->GetVelocity().y});
    }
}

GameObject *CreatePlatform(NetworkInfo network_info, GameObject *wall_left,
                           GameObject *wall_right) {
    GameObject *platform = new GameObject("platform", Controllable);

    platform->SetSize(Size{200, 30});

    Log(LogLevel::Info, "Window width: %d, window height: %d", window_size.width,
        window_size.height);

    Log(LogLevel::Info, "Network id: %d", network_info.id);

    switch (network_info.id) {
    case 1:
        platform->SetPosition({250, float(window_size.height - 40)});
        break;
    case 2:
        platform->SetPosition({250, float(window_size.height - 100)});
        break;
    case 3:
        platform->SetPosition({250, float(window_size.height - 200)});
        break;
    default:
        if (network_info.role != Server) {
            Log(LogLevel::Error, "More than 3 players spotted: EXITING THE GAME. Player ID: %d",
                network_info.id);
            app->quit = true;
        }
    }

    // platform->SetColor(Color{128, 128, 128, 255});
    platform->SetTextureTemplate("assets/platform_{}.png");
    platform->SetOwner(NetworkRole::Client);
    platform->SetCallback([wall_left, wall_right](GameObject *platform) {
        UpdatePlatform(platform, wall_left, wall_right);
    });
    platform->SetAffectedByCollision(false);

    return platform;
}

GameObject *CreateBall() {
    GameObject *ball = new GameObject("ball", Moving);
    ball->SetColor(Color{0, 0, 0, 0});
    ball->SetPosition(Position{200, 200});
    ball->SetSize(Size{50, 50});
    ball->SetAcceleration(Acceleration{0, 0});
    ball->SetVelocity(Velocity{12, 12});
    ball->SetRestitution(1);
    ball->SetTexture("assets/ball.png");
    ball->SetOwner(NetworkRole::Server);
    ball->SetAffectedByCollision(false);

    return ball;
}

std::vector<GameObject *> CreateWall() {
    GameObject *wall_left = new GameObject("wall_left", Stationary);
    GameObject *wall_top = new GameObject("wall_top", Stationary);
    GameObject *wall_right = new GameObject("wall_right", Stationary);

    wall_left->SetColor(Color{255, 0, 0, 255});
    wall_left->SetPosition(Position{0, 0});
    wall_left->SetSize(Size{100, 1080});
    wall_left->SetTexture("assets/wall.jpeg");
    wall_left->SetOwner(NetworkRole::Server);
    wall_left->SetAffectedByCollision(false);

    wall_top->SetColor(Color{255, 0, 0, 255});
    wall_top->SetPosition(Position{100, 0});
    wall_top->SetSize(Size{1820, 100});
    wall_top->SetTexture("assets/wall.jpeg");
    wall_top->SetOwner(NetworkRole::Server);
    wall_top->SetAffectedByCollision(false);

    wall_right->SetColor(Color{255, 0, 0, 255});
    wall_right->SetPosition(Position{1620, 100});
    wall_right->SetSize(Size{100, 980});
    wall_right->SetOwner(NetworkRole::Server);
    wall_right->SetTexture("assets/wall.jpeg");
    wall_top->SetAffectedByCollision(false);

    std::vector<GameObject *> wall_objects = std::vector({wall_left, wall_top, wall_right});

    return wall_objects;
}

std::vector<GameObject *> CreateGameObjects() {
    GameObject *ball = CreateBall();
    std::vector<GameObject *> wall_objects = CreateWall();
    GameObject *wall_left = wall_objects[0];
    GameObject *wall_top = wall_objects[1];
    GameObject *wall_right = wall_objects[2];

    GameObject *platform = CreatePlatform(network_info, wall_left, wall_right);

    std::vector<GameObject *> game_objects =
        std::vector({ball, wall_left, wall_top, wall_right, platform});

    for (GameObject *game_object : game_objects) {
        if (network_info.mode == NetworkMode::PeerToPeer) {
            if (game_object->GetOwner() == NetworkRole::Server) {
                game_object->SetOwner(NetworkRole::Host);
            }
            if (game_object->GetOwner() == NetworkRole::Client) {
                game_object->SetOwner(NetworkRole::Peer);
            }
        }
    }

    return game_objects;
}

void DestroyObjects(std::vector<GameObject *> game_objects) {
    for (GameObject *object : game_objects) {
        delete object;
    }
}

void SetupTimelineInputs(GameEngine *game_engine) {
    // toggle constant and proportional scaling
    app->key_map->key_X.OnPress = []() {
        app->window.proportional_scaling = !app->window.proportional_scaling;
    };
    // toggle pause or unpause
    app->key_map->key_P.OnPress = [game_engine]() { game_engine->BaseTimelineTogglePause(); };
    // slow down the timeline
    app->key_map->key_comma.OnPress = [game_engine]() {
        game_engine->BaseTimelineChangeTic(std::min(game_engine->BaseTimelineGetTic() * 2.0, 2.0));
    };
    // speed up the timeline
    app->key_map->key_period.OnPress = [game_engine]() {
        game_engine->BaseTimelineChangeTic(std::max(game_engine->BaseTimelineGetTic() / 2.0, 0.5));
    };
}

int main(int argc, char *args[]) {
    std::string game_title = "CSC581 HW2 Mitesh's Game: Multiplayer Paddle Game";
    int max_player_count = 3;

    // Initializing the Game Engine
    GameEngine game_engine;
    if (!SetEngineCLIOptions(&game_engine, argc, args)) {
        return 1;
    }

    if (!game_engine.Init()) {
        Log(LogLevel::Error, "Game engine initialization failure");
        return 1;
    }

    SetupTimelineInputs(&game_engine);
    game_engine.SetMaxPlayers(max_player_count);
    game_engine.SetShowPlayerBorder(true);

    network_info = game_engine.GetNetworkInfo();
    if (network_info.id > 3) {
        Log(LogLevel::Error, "More than 3 players spotted: EXITING THE GAME. Player ID: %d",
            network_info.id);
        app->quit = true;
    }

    Color background_color = Color{0, 0, 0, 255};
    game_engine.SetBackgroundColor(background_color);
    game_engine.SetGameTitle(game_title);

    window_size = GetWindowSize();

    std::vector<GameObject *> game_objects = CreateGameObjects();
    game_engine.SetGameObjects(game_objects);
    game_engine.SetCallback(Update);

    // The Start function keeps running until an "exit event occurs"
    game_engine.Start();
    Log(LogLevel::Info, "The game engine has closed the game cleanly");
    // Add Game Cleanup code (deallocating pointers)
    DestroyObjects(game_objects);
    return 0;
}