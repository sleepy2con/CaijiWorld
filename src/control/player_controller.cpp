// Copyright 2026 He Yuxuan. All Rights Reserved.
#include "caijiworld/control/player_controller.h"
#include "caijiworld/core/game.h"
#include "caijiworld/constant/constant.h"
#include <cmath>

namespace caijiworld {

	void PlayerController::MoveIntent(Game* game, int dx, int dy) {
		target_tile_x_ = current_tile_x_ + dx;
		target_tile_y_ = current_tile_y_ + dy;

		if (target_tile_x_ >= 0 && target_tile_x_ < kMapWidth &&
			target_tile_y_ >= 0 && target_tile_y_ < kMapHeight) {
			is_moving_ = true;
		}
	}

	void PlayerController::Update(Game* game, float delta_time) {
		if (!is_moving_) return;

		float target_pixel_x = static_cast<float>(target_tile_x_ * kTileSize);
		float target_pixel_y = static_cast<float>(target_tile_y_ * kTileSize);

		float move_vec_x = target_pixel_x - game->world_->chess_x_;
		float move_vec_y = target_pixel_y - game->world_->chess_y_;

		float distance = std::sqrt(move_vec_x * move_vec_x + move_vec_y * move_vec_y);
		float move_speed = 200.0f;
		float step = move_speed * delta_time;

		if (distance <= step) {
			game->world_->chess_x_ = target_pixel_x;
			game->world_->chess_y_ = target_pixel_y;
			current_tile_x_ = target_tile_x_;
			current_tile_y_ = target_tile_y_;
			is_moving_ = false;
		}
		else {
			game->world_->chess_x_ += (move_vec_x / distance) * step;
			game->world_->chess_y_ += (move_vec_y / distance) * step;
		}
	}

} // namespace caijiworld