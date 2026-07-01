// Copyright 2026 He Yuxuan. All Rights Reserved.
#include "caijiworld/core/game.h"
#include "caijiworld/input/input_manager.h"
#include "caijiworld/control/player_controller.h"
#include "caijiworld/render/viewport.h"
#include <algorithm>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
#include "spdlog/spdlog.h"
#include "caijiworld/constant/constant.h"

namespace caijiworld {

    Game::Game() {}
    Game::~Game() { Shutdown(); }

    bool Game::Initialize(int window_width, int window_height) {
        win_width_ = window_width;
        win_height_ = window_height;

        // SDL_INIT_VIDEO：这是一个宏（实际是 SDL_INIT_VIDEO 标志位），告诉 SDL “我只需要视频功能，音频、手柄、计时器那些别加载，省内存”。
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            spdlog::error("SDL_Init failed: {}", SDL_GetError());
            return false;
        }
        // window_：渲染器绑定的窗口。一个渲染器只能绑定一个窗口（SDL3）
        window_ = SDL_CreateWindow("Caiji World", win_width_, win_height_, 0);
        if (!window_) return false;
        // 渲染器名称，传 nullptr 让 SDL 自动选择最优的后端（你的平台上是 direct3d、opengl、software 中的一个，SDL 内部按优先级选）
        renderer_ = SDL_CreateRenderer(window_, nullptr);
        if (!renderer_) return false;

        // TODO,没明白啥意思
        SDL_SetRenderLogicalPresentation(renderer_, win_width_, win_height_, SDL_LOGICAL_PRESENTATION_LETTERBOX);

        // 初始化 ImGui
        float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        // 编译期检查：当前编译时 ImGui 的版本和 imgui.h 记录的版本是否一致。
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui::GetStyle().ScaleAllSizes(main_scale);
        ImGui::GetStyle().FontScaleDpi = main_scale;

        ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
        ImGui_ImplSDLRenderer3_Init(renderer_);

        // 💡 SSOT：Viewport 先于 InputManager 创建，它是摄像机/缩放数据的唯一权威源。
        // InputManager 只持有 viewport_ 指针消费数据，不存副本。
        viewport_ = std::make_unique<Viewport>(win_width_, win_height_);
        input_manager_ = std::make_unique<InputManager>(viewport_.get());
        player_controller_ = std::make_unique<PlayerController>();
        ui_manager_ = std::make_unique<UiManager>();

        world_ = std::make_unique<World>(kMapWidth, kMapHeight);
        world_->InitWorldData(renderer_);

        return true;
    }

    void Game::Run() {
        Uint64 last_time = SDL_GetTicks();

        while (running_) {
            Uint64 current_time = SDL_GetTicks();
            // delta_time是当前帧距离上一帧，真实世界过去了多少秒（通常是一个零点零几的小数）
            // 电子游戏本质上是一个疯狂翻页的连环画。如果电脑性能好，一秒钟能循环 500 次（500 帧）；如果电脑卡顿，一秒钟只能循环 30 次（30 帧）
            float delta_time = (current_time - last_time) / 1000.0f;
            last_time = current_time;

            // 💡 1. 平台层大循环由 Game 亲自调度
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                // 系统核心级事件：由大总管直接处理
                if (e.type == SDL_EVENT_QUIT) {
                    running_ = false;
                }

                // 游戏输入级事件：喂给输入管理器去消化
                input_manager_->HandleEvent(this, e);
            }

            // 💡 2. 轮询处理键盘持续按下的状态（WASD走格子）
            if (!is_paused_) {
                input_manager_->UpdateKeyboardState(this);
            }

            // 💡 3. 驱动后续的逻辑与渲染
            if (!is_paused_) {
                UpdateLogic(delta_time * game_speed_);
            }
            RenderGame();
        }
    }

    void Game::UpdateLogic(float delta_time) {
        // 1. 先让小人跑自己的物理移动
        player_controller_->Update(this, delta_time);

        // 2. 摄像机继续精确跟着走（缩放数据改用 Viewport 权威源读取）
        float visible_w = win_width_ / viewport_->GetZoom();
        float visible_h = win_height_ / viewport_->GetZoom();
        //  让棋子始终显示在画面正中间，同时限制相机不超出地图边界
        viewport_->SetCameraX(std::clamp(world_->chess_x_ - visible_w / 2.0f, 0.0f, static_cast<float>(kMapWidth * kTileSize - visible_w)));
        viewport_->SetCameraY(std::clamp(world_->chess_y_ - visible_h / 2.0f, 0.0f, static_cast<float>(kMapHeight * kTileSize - visible_h)));

        // 3. 全局世界的自驱动
        world_->Update(delta_time);
    }

    void Game::RenderGame() {
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);

        // 从 SSOT Viewport 读取一次，缓存局部变量避免重复跨模块调用
        // 设置当前画面拍到地图哪个位置，viewport_->GetCameraX()又input设置;
        float cam_x = viewport_->GetCameraX();
        float cam_y = viewport_->GetCameraY();
        float zoom = viewport_->GetZoom();

        // --- 渲染地图 Tile (裁剪) ---
        int start_x = std::max(0, static_cast<int>(cam_x / kTileSize));
        int start_y = std::max(0, static_cast<int>(cam_y / kTileSize));
        int end_x = std::min(kMapWidth, static_cast<int>((cam_x + win_width_) / kTileSize) + 2);
        int end_y = std::min(kMapHeight, static_cast<int>((cam_y + win_height_) / kTileSize) + 2);

        for (int y = start_y; y < end_y; ++y) {
            for (int x = start_x; x < end_x; ++x) {
                SDL_FRect r = { (x * kTileSize - cam_x) * zoom, (y * kTileSize - cam_y) * zoom, kTileSize * zoom, kTileSize * zoom };
                SDL_RenderTexture(renderer_, world_->world_tile_[y][x].tex.get(), nullptr, &r);
            }
        }

        // 渲染选中高亮线框：选中状态从 InputManager（SSOT）读取
        auto selected = input_manager_->GetSelectedCell();
        if (selected.has_value()) {
            SDL_FRect select_rect = { (selected->x * kTileSize - cam_x) * zoom, (selected->y * kTileSize - cam_y) * zoom, kTileSize * zoom, kTileSize * zoom };
            SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
            SDL_RenderRect(renderer_, &select_rect);
        }

        // 渲染主角
        if (world_->chess_texture_) {
            SDL_FRect chess_rect = { (world_->chess_x_ - cam_x) * zoom, (world_->chess_y_ - cam_y) * zoom, kTileSize * zoom, kTileSize * zoom };
            SDL_RenderTexture(renderer_, world_->chess_texture_.get(), nullptr, &chess_rect);
        }

        // 渲染红点小人们
        SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
        for (const auto& u : world_->units_) {
            SDL_FRect r = { (u.x - cam_x) * zoom, (u.y - cam_y) * zoom, 8.0f * zoom, 8.0f * zoom };
            if (r.x + r.w > 0 && r.x < win_width_ && r.y + r.h > 0 && r.y < win_height_) {
                SDL_RenderFillRect(renderer_, &r);
            }
        }

        ui_manager_->Render(this, renderer_);
        SDL_RenderPresent(renderer_);
    }

    void Game::Shutdown() {
        if (renderer_) {
            ImGui_ImplSDLRenderer3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();
            SDL_DestroyRenderer(renderer_);
            renderer_ = nullptr;
        }
        if (window_) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        SDL_Quit();
    }

}  // namespace caijiworld
