// Copyright 2026 He Yuxuan. All Rights Reserved.
#include "caijiworld/ui/ui_manager.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
#include "caijiworld/core/game.h"  // 💡 确保知道 Game 类的具体定义
#include "caijiworld/input/input_manager.h" // 💡 Cancel 按钮需清空 InputManager 中的选中状态

namespace caijiworld {

    void UiManager::Render(Game* game, SDL_Renderer* renderer) {
        // ImGui 需要适配不同的渲染 API，这里用的是 SDL3 的 SDL_Renderer（纯 2D 渲染器）。
        // 这一句内部会重置顶点缓冲区、命令列表等，为接下来的 ImGui 绘制做准备。
        ImGui_ImplSDLRenderer3_NewFrame();
        // 这一句内部会处理鼠标、键盘、游戏手柄等输入设备的上一帧状态，更新 ImGui 的 IO 结构体。
        // 比如鼠标位置、按键状态、滚轮值，都会在这里刷新。
        ImGui_ImplSDL3_NewFrame();
		// 这是 ImGui 本身的帧开始函数。它负责：
		//增加帧计数器
		//	更新 delta time（帧间隔时间）
		//	初始化 UI 堆栈、ID 栈
		//	处理窗口移动 / 缩放请求（从上一帧的鼠标操作产生）
		//	清理上一帧未使用的窗口
        ImGui::NewFrame();

        // UI 组件 1: 右上角时间控制器
        // 强制指定下一个窗口（即将被 Begin 创建的窗口）的位置。按固定值设置Begin窗口位置，
        // ImGuiCond_Always：无条件每帧都应用此位置。换成 ImGuiCond_Once 就只第一次用，之后用户拖走就停在那里；
        // Always 意味着你哪怕用鼠标拖走了，下一帧又被弹回右上角。
        ImGui::SetNextWindowPos(ImVec2(game->win_width_ - 220, 20), ImGuiCond_Always);
        //TODO:没设 SetNextWindowSize，所以窗口的初始大小会由内容自动计算（按钮+间距的紧凑包裹）。后续要做成自适应宽高
		//作用：开始一个名叫 "Time Controls" 的窗口。如果窗口不存在就创建，存在就更新。
		//	参数拆解：
		//	"Time Controls"：窗口标题，显示在标题栏上。
		//	nullptr：指向 bool 的指针，表示关闭按钮是否被按。传 nullptr 表示没有关闭按钮（右上角 X 不会显示，或者显示但点了没用），因为 NoCollapse 和没传 bool* 会共同影响。
		//	ImGuiWindowFlags_NoResize：禁止调整窗口大小。鼠标拖边缘不会出现双箭头，窗口尺寸固定为内容大小。
		//	ImGuiWindowFlags_NoCollapse：禁止折叠。双击标题栏不会把窗口卷成一条标题栏。
        ImGui::Begin("Time Controls", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        {
            if (ImGui::Button(game->is_paused_ ? "▶ Play" : "⏸ Pause")) {
                game->is_paused_ = !game->is_paused_;
            }
			//// 作用：告诉 ImGui：下一个控件不要换行，放在当前行的右边。
			//  细说：没有这一句，ImGui 默认每个控件占一整行。调用后，接下来的 RadioButton 会和 Button 在同一水平线上，从左到右排列。
			//	默认间距：和前一个控件之间会有一个小空格，可通过参数调整。
            ImGui::SameLine();

			//game->game_speed_ == 1：当前是否处于选中状态
            if (ImGui::RadioButton("1x", game->game_speed_ == 1)) { 
                game->game_speed_ = 1;
            }
            ImGui::SameLine();

            if (ImGui::RadioButton("2x", game->game_speed_ == 2)) {
                game->game_speed_ = 2;
            }
        }
        ImGui::End();

        // UI 组件 2: 底部菜单栏
        ImGui::SetNextWindowPos(ImVec2(20, game->win_height_ - 120), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(game->win_width_ - 40, 90), ImGuiCond_Always);
        ImGui::Begin("Architect Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::Text("Orders:");
            ImGui::Separator();

            // 💡 新增：点击这个按钮进入"选中/查看"模式
            if (ImGui::Button("选择", ImVec2(100, 40))) {
                current_tool_ = UiToolType::kSelect;
            }
            ImGui::SameLine();

            if (ImGui::Button("造墙", ImVec2(100, 40))) {
                current_tool_ = UiToolType::kBuildWall;
            }
            ImGui::SameLine();

            if (ImGui::Button("Place Bed", ImVec2(100, 40))) {
                current_tool_ = UiToolType::kPlaceBed;
            }
            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(100, 40))) {
                current_tool_ = UiToolType::kNone;
                // 取消时清空选中状态：selected_cell_ 的权威源在 InputManager，通过 Game 友元访问
                game->input_manager_->ClearSelectedCell();
            }
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    }

}  // namespace caijiworld
