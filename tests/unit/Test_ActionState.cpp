#include <catch2/catch_test_macros.hpp>
#include <SDL3/SDL.h>
#include <fstream>
#include "ze/core/ActionState.h"

TEST_CASE("Silent singleton reset between tests", "[M1-EXT-43][actionstate][singleton]") {
    core::ActionMap::get().setDefaultBindings();
    core::ActionMap::get().resetFrameState();
}

TEST_CASE("Held keys keep MoveForward/MoveBackward true in the same frame", "[M1-EXT-43][actionstate][keyboard]") {
    core::ActionMap::get().setDefaultBindings();
    core::ActionMap::get().resetFrameState();

    SDL_Event wDown = {}; wDown.type = SDL_EVENT_KEY_DOWN; wDown.key.scancode = SDL_SCANCODE_W;
    SDL_Event sDown = {}; sDown.type = SDL_EVENT_KEY_DOWN; sDown.key.scancode = SDL_SCANCODE_S;
    const SDL_Event frame[] = {wDown, sDown};
    core::ActionMap::get().update(frame, 2);

    CHECK(core::ActionMap::get().get(core::Action::MoveForward).held == true);
    CHECK(core::ActionMap::get().get(core::Action::MoveForward).pressed == true);
    CHECK(core::ActionMap::get().get(core::Action::MoveBackward).held == true);
    CHECK(core::ActionMap::get().get(core::Action::MoveBackward).pressed == true);

    core::ActionMap::get().resetFrameState();

    CHECK(core::ActionMap::get().get(core::Action::MoveForward).pressed == false);
    CHECK(core::ActionMap::get().get(core::Action::MoveForward).held == true);
    CHECK(core::ActionMap::get().get(core::Action::MoveBackward).pressed == false);
    CHECK(core::ActionMap::get().get(core::Action::MoveBackward).held == true);
}

TEST_CASE("Released key clears held on key-up", "[M1-EXT-43][actionstate][keyboard][release]") {
    core::ActionMap::get().setDefaultBindings();
    core::ActionMap::get().resetFrameState();

    SDL_Event wDown = {}; wDown.type = SDL_EVENT_KEY_DOWN; wDown.key.scancode = SDL_SCANCODE_W;
    SDL_Event wUp = {}; wUp.type = SDL_EVENT_KEY_UP; wUp.key.scancode = SDL_SCANCODE_W;
    const SDL_Event frame[] = {wDown, wUp};
    core::ActionMap::get().update(frame, 2);

    CHECK(core::ActionMap::get().get(core::Action::MoveForward).released == true);
    CHECK(core::ActionMap::get().get(core::Action::MoveForward).held == false);
}

TEST_CASE("Keyboard press/release translates into action edges", "[M1-EXT-43][actionstate][keyboard]") {
    core::ActionMap::get().setDefaultBindings();
    core::ActionMap::get().resetFrameState();

    // Interact is bound to keyboard E
    SDL_Event down = {}; down.type = SDL_EVENT_KEY_DOWN; down.key.scancode = SDL_SCANCODE_E;
    SDL_Event up = {}; up.type = SDL_EVENT_KEY_UP; up.key.scancode = SDL_SCANCODE_E;
    // Only press in this frame - release in next frame to see the pressed edge
    const SDL_Event frame1[] = {down};
    core::ActionMap::get().update(frame1, 1);

    CHECK(core::ActionMap::get().get(core::Action::Interact).pressed == true);
    CHECK(core::ActionMap::get().get(core::Action::Interact).held == true);
    CHECK(core::ActionMap::get().get(core::Action::Interact).released == false);

    core::ActionMap::get().resetFrameState();

    // Now release
    const SDL_Event frame2[] = {up};
    core::ActionMap::get().update(frame2, 1);

    CHECK(core::ActionMap::get().get(core::Action::Interact).pressed == false);
    CHECK(core::ActionMap::get().get(core::Action::Interact).released == true);
    CHECK(core::ActionMap::get().get(core::Action::Interact).held == false);
}

TEST_CASE("Gamepad axis mapping applies signed deadzone while held", "[M1-EXT-43][actionstate][gamepad]") {
    core::ActionMap::get().setDefaultBindings();
    core::ActionMap::get().setGamepadConnected(true);
    core::ActionMap::get().resetFrameState();

    // MoveForward is bound to LEFTY with negative sign (-1), so negative value = forward
    SDL_Event axis = {};
    axis.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
    axis.gaxis.axis = SDL_GAMEPAD_AXIS_LEFTY;
    axis.gaxis.value = -32767;  // negative = forward (stick up)
    core::ActionMap::get().update(&axis, 1);

    CHECK(core::ActionMap::get().get(core::Action::MoveForward).held == true);
    CHECK(core::ActionMap::get().get(core::Action::MoveForward).analogValue != 0.0f);

    core::ActionMap::get().resetFrameState();

    CHECK(core::ActionMap::get().get(core::Action::MoveForward).held == true);
}

TEST_CASE("ActionMap save writes versioned text bindings file", "[M1-EXT-45][actionstate][persistence]") {
    core::ActionMap::get().setDefaultBindings();

    const char* path = "test_action_bindings.txt";
    core::ActionMap::get().save(path);

    std::ifstream file(path);
    REQUIRE(file.is_open());
    std::string header;
    std::getline(file, header);
    CHECK(header == "ZE_BINDINGS_V1");
}

TEST_CASE("ActionMap load falls back to defaults when file is missing", "[M1-EXT-45][actionstate][persistence]") {
    core::ActionMap::get().setDefaultBindings();
    core::ActionMap::get().load("__missing_action_map__.txt");
    CHECK(core::ActionMap::get().get(core::Action::Jump).analogValue == 0.0f);
}

TEST_CASE("resetFrameState clears edge flags without clearing held", "[M1-EXT-43][actionstate][frame]") {
    core::ActionMap::get().setDefaultBindings();
    core::ActionMap::get().resetFrameState();

    SDL_Event down = {}; down.type = SDL_EVENT_KEY_DOWN; down.key.scancode = SDL_SCANCODE_E;
    core::ActionMap::get().update(&down, 1);

    CHECK(core::ActionMap::get().get(core::Action::Interact).pressed == true);
    CHECK(core::ActionMap::get().get(core::Action::Interact).held == true);

    core::ActionMap::get().resetFrameState();

    CHECK(core::ActionMap::get().get(core::Action::Interact).pressed == false);
    CHECK(core::ActionMap::get().get(core::Action::Interact).released == false);
    CHECK(core::ActionMap::get().get(core::Action::Interact).held == true);
}