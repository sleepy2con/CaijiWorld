// Copyright 2026 He Yuxuan. All Rights Reserved.
#ifndef CAIJIWORLD_CORE_GAME_H_
#define CAIJIWORLD_CORE_GAME_H_

#include <SDL3/SDL.h>
#include <memory>
#include <string>

#include "world/world.h"

namespace caijiworld {

	// 抽象出当前 UI 的选择状态，方便逻辑层读取
	enum class UiToolType {
		kNone,
		kBuildWall,
		kPlaceBed,
		kHarvest
	};

	class Game {
	public:
		Game();
		~Game();

		// 禁止拷贝和移动
		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;

		bool Initialize(int window_width, int window_height);
		void Run();
		void Shutdown();

	private:
		void HandleEvents(float delta_time);
		void UpdateLogic(float delta_time);
		void RenderGame();
		void RenderUi();  // 💡 所有的 ImGui UI 组件全部收纳在这里处理

		bool running_ = true;
		int win_width_ = 1200;
		int win_height_ = 900;
		float zoom_level_ = 1.0f;

		// RimWorld 核心玩法相关的状态变量
		bool is_paused_ = false;
		int game_speed_ = 1;
		UiToolType current_tool_ = UiToolType::kNone;

		// 摄像机坐标
		float camera_x_ = 0.0f;
		float camera_y_ = 0.0f;

		// SDL 资源指针
		SDL_Window* window_ = nullptr;
		SDL_Renderer* renderer_ = nullptr;

		// 游戏世界
		std::unique_ptr<World> world_;
	};

}  // namespace caijiworld

#endif  // CAIJIWORLD_CORE_GAME_H_