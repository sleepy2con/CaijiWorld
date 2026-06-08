// Copyright 2026 He Yuxuan. All Rights Reserved.
#include "core/game.h"

#include <algorithm>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
#include "spdlog/spdlog.h"
#include "constant/constant.h"

namespace caijiworld {

	Game::Game() {}

	Game::~Game() {
		Shutdown();
	}

	bool Game::Initialize(int window_width, int window_height) {
		win_width_ = window_width;
		win_height_ = window_height;

		if (!SDL_Init(SDL_INIT_VIDEO)) {
			spdlog::error("SDL_Init failed: {}", SDL_GetError());
			return false;
		}

		window_ = SDL_CreateWindow("Caiji World", win_width_, win_height_, 0);
		if (!window_) return false;

		renderer_ = SDL_CreateRenderer(window_, nullptr);
		if (!renderer_) return false;

		SDL_SetRenderLogicalPresentation(renderer_, win_width_, win_height_,
			SDL_LOGICAL_PRESENTATION_LETTERBOX);

		// 初始化 ImGui
		float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetStyle().ScaleAllSizes(main_scale);
		ImGui::GetStyle().FontScaleDpi = main_scale;

		ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
		ImGui_ImplSDLRenderer3_Init(renderer_);

		// 创建世界并载入数据
		world_ = std::make_unique<World>(kMapWidth, kMapHeight);
		world_->InitWorldData(renderer_);

		return true;
	}
	// 游戏引擎是“无论有没有事件，我都必须死循环，拼命刷新画面”
	void Game::Run() {
		Uint64 last_time = SDL_GetTicks();

		while (running_) {
			Uint64 current_time = SDL_GetTicks();
			float delta_time = (current_time - last_time) / 1000.0f;
			last_time = current_time;

			HandleEvents(delta_time);

			if (!is_paused_) {
				UpdateLogic(delta_time * game_speed_);
			}

			RenderGame();
		}
	}
	// delta_time是当前帧距离上一帧，真实世界过去了多少秒（通常是一个零点零几的小数）
	// 电子游戏本质上是一个疯狂翻页的连环画。如果电脑性能好，一秒钟能循环 500 次（500 帧）；如果电脑卡顿，一秒钟只能循环 30 次（30 帧）
	void Game::HandleEvents(float delta_time) {
		SDL_Event e;
		// 这里的死循环会永久卡死吗？
		// 绝对不会。 因为 SDL_PollEvent 是非阻塞（Non - blocking）的。当这一帧所有的积压事件都被弹出、队列变空后，
		// 它就会返回 false，程序流程立刻向下走，去执行 UpdateLogic 和 RenderGame。
		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL3_ProcessEvent(&e);

			if (e.type == SDL_EVENT_QUIT) running_ = false;
			if (e.type == SDL_EVENT_MOUSE_WHEEL) {
				zoom_level_ += e.wheel.y * 0.1f;
				zoom_level_ = std::clamp(zoom_level_, 0.3f, 3.0f);
			}

			// 💡 演示：如何处理 UI 状态与鼠标点击世界格子的交互
			if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
				// 如果当前鼠标点在了 ImGui 的 UI 窗口上，不要触发游戏世界的建造
				if (ImGui::GetIO().WantCaptureMouse) {
					continue;
				}

				// 计算点击位置对应的游戏世界坐标
				float mouse_x = e.button.x;
				float mouse_y = e.button.y;
				float visible_w = win_width_ / zoom_level_;
				float visible_h = win_height_ / zoom_level_;

				float world_click_x = camera_x_ + (mouse_x / win_width_) * visible_w;
				float world_click_y = camera_y_ + (mouse_y / win_height_) * visible_h;

				int tile_x = static_cast<int>(world_click_x / kTileSize);
				int tile_y = static_cast<int>(world_click_y / kTileSize);

				// 根据 UI 选中的工具执行建造
				if (current_tool_ == UiToolType::kBuildWall) {
					spdlog::info("UI Command: Build wall at grid ({}, {})", tile_x, tile_y);
					// world_->BuildWallAt(tile_x, tile_y); // 实际调用世界格子更新
				}
			}
		}

		// 键盘控制玩家移动
		// 💡 键盘控制小人走格子
		// 什么是“平滑走格子”？
			//小人不能像以前那样在格子里随意停在任何像素点。他的状态应该只有两种：
			//静止状态：老老实实呆在某个格子正中心（比如第 x 行，第 y 列）。
			//移动状态：按下方向键后，锁定一个目标格子（可以是上下左右，也可以是斜方向），然后花零点几秒平滑地“滑”过去。在到达目标之前，不接收新的输入。
		if (!is_paused_) {
			// 如果小人正在两个格子之间移动，直接返回，让他专心把当前的格子走完
			if (is_moving_) return;

			const bool* keys = SDL_GetKeyboardState(nullptr);
			int dx = 0;
			int dy = 0;

			// 检测四个方向，允许组合（如 W+D 使得 dx=1, dy=-1，即斜右上）
			if (keys[SDL_SCANCODE_W]) dy -= 1;
			if (keys[SDL_SCANCODE_S]) dy += 1;
			if (keys[SDL_SCANCODE_A]) dx -= 1;
			if (keys[SDL_SCANCODE_D]) dx += 1;

			// 如果玩家按下了方向键
			if (dx != 0 || dy != 0) {
				target_tile_x_ = current_tile_x_ + dx;
				target_tile_y_ = current_tile_y_ + dy;

				// 边界检查：确保目标格子没有走出地图边界
				if (target_tile_x_ >= 0 && target_tile_x_ < kMapWidth &&
					target_tile_y_ >= 0 && target_tile_y_ < kMapHeight) {
					is_moving_ = true;  // 激活移动状态！
				}
			}
		}
	}

	void Game::UpdateLogic(float delta_time) {
		if (is_moving_) {
			// 将目标网格坐标转换成真实的像素坐标
			float target_pixel_x = static_cast<float>(target_tile_x_ * kTileSize);
			float target_pixel_y = static_cast<float>(target_tile_y_ * kTileSize);

			// 计算移动方向向量
			float move_vec_x = target_pixel_x - world_->chess_x_;
			float move_vec_y = target_pixel_y - world_->chess_y_;

			// 计算剩余距离
			float distance = std::sqrt(move_vec_x * move_vec_x + move_vec_y * move_vec_y);

			float move_speed = 200.0f;  // 小人移动速度（像素/秒）
			float step = move_speed * delta_time;

			if (distance <= step) {
				// 💡 如果这一帧走过去的距离已经直接能到达或超过目标点，说明到站了
				world_->chess_x_ = target_pixel_x;
				world_->chess_y_ = target_pixel_y;
				current_tile_x_ = target_tile_x_;
				current_tile_y_ = target_tile_y_;
				is_moving_ = false;  // 关闭移动状态，允许接收下一次键盘输入
			}
			else {
				// 💡 否则，沿着方向向量规规矩矩地前进
				world_->chess_x_ += (move_vec_x / distance) * step;
				world_->chess_y_ += (move_vec_y / distance) * step;
			}
		}

		// 保持摄像机跟随 (保持你原本的代码不变)
		float visible_w = win_width_ / zoom_level_;
		float visible_h = win_height_ / zoom_level_;
		camera_x_ = std::clamp(world_->chess_x_ - visible_w / 2.0f, 0.0f,
			static_cast<float>(kMapWidth * kTileSize - visible_w));
		camera_y_ = std::clamp(world_->chess_y_ - visible_h / 2.0f, 0.0f,
			static_cast<float>(kMapHeight * kTileSize - visible_h));

		world_->Update(delta_time);
	}

	void Game::RenderGame() {
		SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
		SDL_RenderClear(renderer_);

		// --- 渲染地图 Tile (沿用你之前的裁剪逻辑) ---
		int start_x = std::max(0, static_cast<int>(camera_x_ / kTileSize));
		int start_y = std::max(0, static_cast<int>(camera_y_ / kTileSize));
		int end_x = std::min(kMapWidth, static_cast<int>((camera_x_ + win_width_) / kTileSize) + 2);
		int end_y = std::min(kMapHeight, static_cast<int>((camera_y_ + win_height_) / kTileSize) + 2);

		for (int y = start_y; y < end_y; ++y) {
			for (int x = start_x; x < end_x; ++x) {
				SDL_FRect r = { (x * kTileSize - camera_x_) * zoom_level_,
								(y * kTileSize - camera_y_) * zoom_level_,
								kTileSize * zoom_level_, kTileSize * zoom_level_ };
				SDL_RenderTexture(renderer_, world_->world_tile_[y][x].tex.get(), nullptr, &r);
			}
		}

		// 2. 💡 检查这里：渲染玩家控制的主棋子（Chess）
		if (world_->chess_texture_) {
			SDL_FRect chess_rect = {
				(world_->chess_x_ - camera_x_) * zoom_level_,
				(world_->chess_y_ - camera_y_) * zoom_level_,
				kTileSize * zoom_level_,
				kTileSize * zoom_level_
			};
			SDL_RenderTexture(renderer_, world_->chess_texture_.get(), nullptr, &chess_rect);
		}

		// 3. 💡 检查这里：渲染那群随机乱走的红点小人（Units）
		SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255); // 设为红色
		for (const auto& u : world_->units_) {
			// 同样需要计算摄像机偏移和缩放
			SDL_FRect r = {
				(u.x - camera_x_) * zoom_level_,
				(u.y - camera_y_) * zoom_level_,
				8.0f * zoom_level_,
				8.0f * zoom_level_
			};

			// 边界剔除：在屏幕内才绘制
			if (r.x + r.w > 0 && r.x < win_width_ && r.y + r.h > 0 && r.y < win_height_) {
				SDL_RenderFillRect(renderer_, &r);
			}
		}

		// --- 绘制 UI 界面 ---
		RenderUi();

		SDL_RenderPresent(renderer_);
	}

	void Game::RenderUi() {
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// 💡 以后你想加什么 UI 组件，直接在这个函数里开辟新窗口加！非常安全。

		// UI 组件 1: 仿 RimWorld 右上角时间控制器
		ImGui::SetNextWindowPos(ImVec2(win_width_ - 220, 20), ImGuiCond_Always);
		ImGui::Begin("Time Controls", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		{
			if (ImGui::Button(is_paused_ ? "▶ Play" : "⏸ Pause")) is_paused_ = !is_paused_;
			ImGui::SameLine();
			if (ImGui::RadioButton("1x", game_speed_ == 1)) game_speed_ = 1; ImGui::SameLine();
			if (ImGui::RadioButton("2x", game_speed_ == 2)) game_speed_ = 2; ImGui::SameLine();
			if (ImGui::RadioButton("4x", game_speed_ == 4)) game_speed_ = 4;
		}
		ImGui::End();

		// UI 组件 2: 仿 RimWorld 底部菜单栏
		ImGui::SetNextWindowPos(ImVec2(20, win_height_ - 120), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(win_width_ - 40, 90), ImGuiCond_Always);
		ImGui::Begin("Architect Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		{
			ImGui::Text("Orders:");
			ImGui::Separator();
			if (ImGui::Button("Build Wall", ImVec2(100, 40))) current_tool_ = UiToolType::kBuildWall; ImGui::SameLine();
			if (ImGui::Button("Place Bed", ImVec2(100, 40))) current_tool_ = UiToolType::kPlaceBed; ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(100, 40))) current_tool_ = UiToolType::kNone;
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
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