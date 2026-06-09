// Copyright 2026 He Yuxuan. All Rights Reserved.
#ifndef CAIJIWORLD_UI_UI_MANAGER_H_
#define CAIJIWORLD_UI_UI_MANAGER_H_

#include <SDL3/SDL.h>

namespace caijiworld {

	// 前置声明 Game 类，避免循环引用
	class Game;

	class UiManager {
	public:
		UiManager() = default;
		~UiManager() = default;

		// 禁止拷贝
		UiManager(const UiManager&) = delete;
		UiManager& operator=(const UiManager&) = delete;

		// 💡 核心：每帧把 Game 的指针传进来，让 UI 知道去画什么，以及点击按钮时修改谁
		void Render(Game* game, SDL_Renderer* renderer);
	};

}  // namespace caijiworld

#endif  // CAIJIWORLD_UI_UI_MANAGER_H_