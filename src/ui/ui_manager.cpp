// Copyright 2026 He Yuxuan. All Rights Reserved.
#include "caijiworld/ui/ui_manager.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
#include "caijiworld/core/game.h"  // 💡 确保知道 Game 类的具体定义

namespace caijiworld {

    void UiManager::Render(Game* game, SDL_Renderer* renderer) {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // UI 组件 1: 右上角时间控制器
        ImGui::SetNextWindowPos(ImVec2(game->win_width_ - 220, 20), ImGuiCond_Always);
        ImGui::Begin("Time Controls", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        {
            if (ImGui::Button(game->is_paused_ ? "▶ Play" : "⏸ Pause")) {
                game->is_paused_ = !game->is_paused_;
            }
            ImGui::SameLine();

            // 💡 严格规范换行，消除编译器的歧义
            if (ImGui::RadioButton("1x", game->game_speed_ == 1)) {
                game->game_speed_ = 1;
            }
            ImGui::SameLine();

            if (ImGui::RadioButton("2x", game->game_speed_ == 2)) {
                game->game_speed_ = 2;
            }
            ImGui::SameLine();

            if (ImGui::RadioButton("4x", game->game_speed_ == 4)) {
                game->game_speed_ = 4;
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

            if (ImGui::Button("Build Wall", ImVec2(100, 40))) {
                game->current_tool_ = UiToolType::kBuildWall;
            }
            ImGui::SameLine();

            if (ImGui::Button("Place Bed", ImVec2(100, 40))) {
                game->current_tool_ = UiToolType::kPlaceBed;
            }
            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(100, 40))) {
                game->current_tool_ = UiToolType::kNone;
            }
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    }

}  // namespace caijiworld