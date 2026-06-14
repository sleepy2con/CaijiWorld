// Copyright 2026 He Yuxuan. All Rights Reserved.
#ifndef CAIJIWORLD_CORE_GAME_H_
#define CAIJIWORLD_CORE_GAME_H_

#include <SDL3/SDL.h>
#include <memory>
#include <string>

#include "caijiworld/world/world.h"
#include "caijiworld/ui/ui_manager.h" // 💡 包含新头文件

namespace caijiworld {

	class Game {
		// 💡 声明友元，让 UiManager 可以直接读写 Game 的私有变量，省去写一堆 Getter/Setter
		friend class UiManager;
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

		// 💡 新增 UI 管理器指针
		std::unique_ptr<UiManager> ui_manager_;

		// 游戏世界
		std::unique_ptr<World> world_;

		// 💡 针对 RimWorld 八方向走格子新增的控制变量
		int current_tile_x_ = 0;  // 小人当前所在的格子 X 坐标
		int current_tile_y_ = 0;  // 小人当前所在的格子 Y 坐标
		int target_tile_x_ = 0;   // 小人的目标格子 X 坐标
		int target_tile_y_ = 0;   // 小人的目标格子 Y 坐标
		bool is_moving_ = false;  // 小人当前是否正在网格间移动

		int selected_tile_x_ = -1;  // 👈 当前选中的格子X坐标
		int selected_tile_y_ = -1;  // 👈 当前选中的格子Y坐标
	};

}  // namespace caijiworld

#endif  // CAIJIWORLD_CORE_GAME_H_