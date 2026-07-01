// Copyright 2026 He Yuxuan. All Rights Reserved.
#ifndef CAIJIWORLD_INPUT_INPUT_MANAGER_H_
#define CAIJIWORLD_INPUT_INPUT_MANAGER_H_

#include <SDL3/SDL.h> // 👈 必须包含，确保编译器在.h里就认得所有 SDL_Event 子类型
#include <optional>

namespace caijiworld {

    class Game;
    class Viewport;

    class InputManager {
    public:
        // SSOT：InputManager 不持有 zoom/camera 的副本，必须通过 viewport_ 指针读写。
        // 缩放和视口数据由 Viewport 一手管理，InputManager 只是"使用"它们。
        explicit InputManager(Viewport* viewport);
        ~InputManager() = default;

        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        // 核心单次事件分发
        void HandleEvent(Game* game, const SDL_Event& e);

        // 每帧键盘长按状态更新
        void UpdateKeyboardState(Game* game);

        // SSOT：选中的地图格子坐标（左上角为 0,0）。由鼠标点击时通过 Viewport::ScreenToWorld
        // 计算得出并存在这里。std::nullopt 表示没有选中。InputManager 是选中状态的唯一权威源。
        [[nodiscard]] std::optional<SDL_Point> GetSelectedCell() const { return selected_cell_; }
        void ClearSelectedCell() { selected_cell_ = std::nullopt; }

    private:
        // 💡 viewport_ 是非拥有指针，Viewport 的生命周期由 Game 管理。
        // InputManager 只是 Viewport 的"消费者"，不拥有它。
        Viewport* viewport_ = nullptr;

        // 当前选中的地图格子，鼠标点击时通过视口坐标转换计算得出
        std::optional<SDL_Point> selected_cell_;

        // 拖拽平移相关状态
        bool is_dragging_ = false;
        float last_mouse_x_ = 0.0f;
        float last_mouse_y_ = 0.0f;

        static constexpr float kZoomSensitivity = 0.1f;
    };

} // namespace caijiworld
#endif // CAIJIWORLD_INPUT_INPUT_MANAGER_H_
