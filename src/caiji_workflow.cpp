// Copyright 2026 He Yuxuan. All Rights Reserved.
#include "core/game.h"
#include "constant/constant.h"
#include "spdlog/spdlog.h"

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Initializing CaijiWorld Engine...");

    caijiworld::Game game;
    if (!game.Initialize(kWindowWidth, kWindowHeight)) {
        spdlog::critical("Failed to initialize game engine!");
        return -1;
    }

    spdlog::info("Engine running successfully.");
    game.Run();

    return 0;
}
