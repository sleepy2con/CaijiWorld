#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <memory>
#include "spdlog/spdlog.h"
//#include "spdlog/cfg/env.h"   // support for loading levels from the environment variable
//#include "spdlog/fmt/ostr.h"  // support for user defined types

const int TILE_SIZE = 16;
const int MAP_W = 300;
const int MAP_H = 300;
const int SCREEN_W = 800;
const int SCREEN_H = 600;

struct Tile {
	std::shared_ptr<SDL_Texture> tex;
};

struct Unit {
	float x, y;
};

int main(int argc, char* argv[])
{
	spdlog::set_level(spdlog::level::debug);  // Set global log level to info
	spdlog::debug("hello the game!");

	SDL_Init(SDL_INIT_VIDEO);

	std::unique_ptr <SDL_Window, decltype(&SDL_DestroyWindow)> window(SDL_CreateWindow("300x300 Grid Demo", SCREEN_W, SCREEN_H, 0), SDL_DestroyWindow);
	std::unique_ptr <SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(SDL_CreateRenderer(window.get(), nullptr), SDL_DestroyRenderer);
	// decltype 解析地址类型，相当于得到当前函数指针类型
	std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surf_grass(IMG_Load("../../assets/pics/grass.png"), SDL_DestroySurface);
	if (!surf_grass) {
		spdlog::error("load water pic failed");
		return -1;
	}

	std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surf_water(IMG_Load("../../assets/pics/water.png"), SDL_DestroySurface);
	if (!surf_water) {
		spdlog::error("load water pic failed");
		return -1;
	}
	if (!surf_grass || !surf_water) {
		spdlog::error("Texture load failed: %s", SDL_GetError());
	}

	std::shared_ptr<SDL_Texture>tex_grass(SDL_CreateTextureFromSurface(renderer.get(), surf_grass.get()), SDL_DestroyTexture);
	std::shared_ptr<SDL_Texture>tex_water(SDL_CreateTextureFromSurface(renderer.get(), surf_water.get()), SDL_DestroyTexture);

	// 地图创建
	std::vector<std::vector<Tile>> map(MAP_H, std::vector<Tile>(MAP_W));
	srand((unsigned int)time(nullptr));
	for (int y = 0; y < MAP_H; y++) {
		for (int x = 0; x < MAP_W; x++) {
			map[y][x].tex = (rand() % 5 == 0) ? tex_water : tex_grass;
		}
	}

	std::vector<Unit> units(100);
	for (auto& u : units) {
		u.x = (float)(rand() % (MAP_W * TILE_SIZE));
		u.y = (float)(rand() % (MAP_H * TILE_SIZE));
	}

	float camX = 0, camY = 0;
	bool running = true;
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT) running = false;
		}

		for (auto& u : units) {
			u.x += ((rand() % 3) - 1) * 0.5f;
			u.y += ((rand() % 3) - 1) * 0.5f;
		}

		SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
		SDL_RenderClear(renderer.get());

		// 视口裁剪渲染
		int startX = (int)(camX / TILE_SIZE);
		int startY = (int)(camY / TILE_SIZE);
		int endX = (int)((camX + SCREEN_W) / TILE_SIZE + 1);
		int endY = (int)((camY + SCREEN_H) / TILE_SIZE + 1);

		// 越界检查
		if (startX < 0) startX = 0;
		if (startY < 0) startY = 0;

		for (int y = startY; y < endY && y < MAP_H; y++) {
			for (int x = startX; x < endX && x < MAP_W; x++) {
				SDL_FRect r = { (float)x * TILE_SIZE - camX, (float)y * TILE_SIZE - camY, (float)TILE_SIZE, (float)TILE_SIZE };
				SDL_RenderTexture(renderer.get(), map[y][x].tex.get(), NULL, &r);
			}
		}

		SDL_SetRenderDrawColor(renderer.get(), 255, 0, 0, 255);
		for (auto& u : units) {
			SDL_FRect r = { u.x - camX, u.y - camY, 8.0f, 8.0f };
			SDL_RenderFillRect(renderer.get(), &r);
		}

		SDL_RenderPresent(renderer.get());
	}

	SDL_Quit();
	return 0;
}