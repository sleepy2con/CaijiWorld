// Copyright 2026 He Yuxuan. All Rights Reserved.
#ifndef CAIJIWORLD_UI_UI_MANAGER_H_
#define CAIJIWORLD_UI_UI_MANAGER_H_

#include <SDL3/SDL.h>

namespace caijiworld {

    // 抽象出当前 UI 的选择状态，方便逻辑层读取
    // 目前暂时打算根据改状态枚举去判定区分当前的ui btn
    enum class UiToolType {
        kNone,
        kBuildWall,
        kPlaceBed,
        kSelect       // 👈 新增：选中/查看模式
    };

    // 前置声明 Game 类，避免循环引用
    class Game;

    class UiManager {
    public:
        UiManager() = default;
        ~UiManager() = default;

        UiManager(const UiManager&) = delete;
        UiManager& operator=(const UiManager&) = delete;

        // 💡 核心：每帧把 Game 的指针传进来，让 UI 知道去画什么，以及点击按钮时修改谁
        void Render(Game* game, SDL_Renderer* renderer);

        // SSOT：当前激活的工具由 UI 面板按钮一手决定，UiManager 是唯一权威源。
        // InputManager 只读取不存储，Game 不再持有 current_tool_ 副本。
        [[nodiscard]] UiToolType GetCurrentTool() const { return current_tool_; }
        void SetCurrentTool(UiToolType tool) { current_tool_ = tool; }

    private:
        // 当前激活的工具栏工具，点击底部菜单按钮时切换
        UiToolType current_tool_ = UiToolType::kNone;

        // 面板显隐状态（后续可绑定快捷键或菜单开关）
        bool show_time_controls_ = true;
        bool show_architect_menu_ = true;
    };

}  // namespace caijiworld

#endif  // CAIJIWORLD_UI_UI_MANAGER_H_
