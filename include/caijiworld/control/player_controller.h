// Copyright 2026 He Yuxuan. All Rights Reserved.
#ifndef CAIJIWORLD_CONTROL_PLAYER_CONTROLLER_H_
#define CAIJIWORLD_CONTROL_PLAYER_CONTROLLER_H_

namespace caijiworld {

	class Game;

	class PlayerController {
	public:
		PlayerController() = default;
		~PlayerController() = default;

		// 更新控制小人的移动逻辑（对应你原本写在 UpdateLogic 里的部分）
		void Update(Game* game, float delta_time);

		// 外部输入通知：玩家下达了走格子的相对意图
		void MoveIntent(Game* game, int dx, int dy);

		// 暴露必要的 Getter 给 Game 渲染或摄像机跟随使用
		bool IsMoving() const { return is_moving_; }
		int GetCurrentTileX() const { return current_tile_x_; }
		int GetCurrentTileY() const { return current_tile_y_; }

	private:
		// 将原 Game 里的控制变量完美收容到这里
		int current_tile_x_ = 0;
		int current_tile_y_ = 0;
		int target_tile_x_ = 0;
		int target_tile_y_ = 0;
		bool is_moving_ = false;
	};

} // namespace caijiworld
#endif // CAIJIWORLD_CONTROL_PLAYER_CONTROLLER_H_