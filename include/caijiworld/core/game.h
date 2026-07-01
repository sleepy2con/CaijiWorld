// Copyright 2026 He Yuxuan. All Rights Reserved.
#ifndef CAIJIWORLD_CORE_GAME_H_
#define CAIJIWORLD_CORE_GAME_H_

#include <SDL3/SDL.h>
#include <memory>
#include "caijiworld/world/world.h"
#include "caijiworld/ui/ui_manager.h"

namespace caijiworld {

    // 💡 干净的前置声明体系
    class InputManager;
    class PlayerController;
    class Viewport;

    class Game {
        // 允许子系统直接读写，避免繁琐的 getter/setter
        friend class UiManager;
        friend class PlayerController;
        friend class InputManager; // 👈
    public:
        Game();
        ~Game();

        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;

        bool Initialize(int window_width, int window_height);
        void Run();
        void Shutdown();

    private:
        void UpdateLogic(float delta_time);
        void RenderGame();

        // --------------------------基础核心上下文数据
        bool running_ = true;

        int win_width_{};	// 记录传递进来的constant里的kWindowWidth的值，用于SDL库创建当前的主进程窗口大小
        int win_height_{};


        bool is_paused_ = false;
        int game_speed_{ 1 };


        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;

        // 💡 模块化解耦后的四大系统组件
        // Viewport 是摄像机位置和缩放因子的唯一权威源（SSOT），InputManager 和渲染都通过它读写
        std::unique_ptr<Viewport> viewport_;
        std::unique_ptr<InputManager> input_manager_;
        std::unique_ptr<PlayerController> player_controller_;
        std::unique_ptr<UiManager> ui_manager_;
        std::unique_ptr<World> world_;
    };

}  // namespace caijiworld
#endif  // CAIJIWORLD_CORE_GAME_H_
