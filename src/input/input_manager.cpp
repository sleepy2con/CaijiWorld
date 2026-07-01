// Copyright 2026 He Yuxuan. All Rights Reserved.
#include "caijiworld/input/input_manager.h"
#include "caijiworld/core/game.h"
#include "caijiworld/control/player_controller.h"
#include "caijiworld/render/viewport.h" // 💡 引入新视口头文件
#include "caijiworld/ui/ui_manager.h"  // 💡 读取当前激活的工具 (current_tool_)
#include "imgui/imgui.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <imgui_impl_sdl3.h>
#include <caijiworld/constant/constant.h>

namespace caijiworld {

    InputManager::InputManager(Viewport* viewport)
        : viewport_(viewport) {
    }

    void InputManager::HandleEvent(Game* game, const SDL_Event& e) {
        // 1. 让 ImGui 优先拦截并处理 UI 事件（防止穿透到游戏世界）
        ImGui_ImplSDL3_ProcessEvent(&e);

        // 2. 纯粹的游戏业务事件分发
        switch (e.type) {
        // --- 滚轮缩放 ---
        case SDL_EVENT_MOUSE_WHEEL:
            // ImGui 想独占鼠标 → 不缩放地图（用户在操作 ImGui 面板）
            if (ImGui::GetIO().WantCaptureMouse) {
                break;
            }
            viewport_->AddZoom(e.wheel.y * kZoomSensitivity);
            spdlog::debug("Scroll detected. Zoom level updated to: {}", viewport_->GetZoom());
            break;

        // --- 鼠标按下 ---
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (e.button.button == SDL_BUTTON_LEFT) {
                // ImGui 想独占鼠标 → 不处理游戏世界点击
                if (ImGui::GetIO().WantCaptureMouse) {
                    break;
                }
                // 记录拖拽起点，准备可能的平移操作
                is_dragging_ = true;
                last_mouse_x_ = e.button.x;
                last_mouse_y_ = e.button.y;

                // 选择模式下：计算当前鼠标指向的地图格子并存储选中
                if (game->ui_manager_->GetCurrentTool() == UiToolType::kSelect) {
                    SDL_Point tile = viewport_->ScreenToWorld(e.button.x, e.button.y);
                    if (tile.x >= 0 && tile.x < kMapWidth && tile.y >= 0 && tile.y < kMapHeight) {
                        selected_cell_ = tile;
                        spdlog::info("Grid Selected via InputManager: ({}, {})", tile.x, tile.y);
                    }
                }
                // ... 后面加建造墙壁等逻辑
            }
            break;

        // --- 鼠标移动 ---
        case SDL_EVENT_MOUSE_MOTION:
            // 拖拽中不检查 WantCaptureMouse：拖拽起点在游戏区域，用户可能把鼠标
            // 拖到面板上方，此时仍应继续平移而不是突然中断
            if (is_dragging_) {
                float dx = e.motion.x - last_mouse_x_;
                float dy = e.motion.y - last_mouse_y_;
                // 屏幕像素位移换算为世界坐标位移（反向：鼠标右移 → 世界左移）
                float zoom = viewport_->GetZoom();
                if (zoom > 0.0f) {
                    viewport_->MoveCamera(-dx / zoom, -dy / zoom);
                }
                last_mouse_x_ = e.motion.x;
                last_mouse_y_ = e.motion.y;
            }
            break;

        // --- 鼠标松开 ---
        case SDL_EVENT_MOUSE_BUTTON_UP:
            // 即使鼠标已移到 ImGui 区域上也要处理松开，否则拖拽状态会"粘住"
            if (e.button.button == SDL_BUTTON_LEFT && is_dragging_) {
                is_dragging_ = false;
            }
            break;

        default:
            break;
        }
    }

    void InputManager::UpdateKeyboardState(Game* game) {
        // 如果小人正在格子间平滑滑动，锁定输入
        if (game->player_controller_->IsMoving()) return;

        const bool* keys = SDL_GetKeyboardState(nullptr);
        int dx = 0, dy = 0;

        if (keys[SDL_SCANCODE_W]) dy -= 1;
        if (keys[SDL_SCANCODE_S]) dy += 1;
        if (keys[SDL_SCANCODE_A]) dx -= 1;
        if (keys[SDL_SCANCODE_D]) dx += 1;

        if (dx != 0 || dy != 0) {
            // 把移动意图丢给控制器
            game->player_controller_->MoveIntent(game, dx, dy);
        }
    }

} // namespace caijiworld
