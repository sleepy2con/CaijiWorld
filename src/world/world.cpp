
#include "world/world.h" // 1st
#include "constant/constant.h"
#include "world/tile.h"

#include "spdlog/spdlog.h" // 3st


#include <SDL3_image/SDL_image.h>

#include <vector>	// 4st
#include <cstdlib>
#include <ctime>
#include <memory>


World::World(int width_, int height_) :m_world(height_, std::vector<Tile>(width_)), m_units(1), m_window(SDL_CreateWindow("300x300 Grid Demo", SCREEN_W, SCREEN_H, 0), SDL_DestroyWindow),
m_renderer(SDL_CreateRenderer(m_window.get(), nullptr), SDL_DestroyRenderer)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		spdlog::error("SDL_Init failed: {}", SDL_GetError());
		return;
	}

	const std::string _title_str{ "caijiworld_" + std::to_string(width_) + "*" + std::to_string(height_) };
	// 在函数体内部进行真正的窗口创建
	m_window.reset(SDL_CreateWindow(_title_str.c_str(), SCREEN_W, SCREEN_H, 0));
	m_renderer.reset(SDL_CreateRenderer(m_window.get(), nullptr));

	// decltype 解析地址类型，相当于得到当前函数指针类型
	std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surf_grass(IMG_Load("../../assets/pics/grass.png"), SDL_DestroySurface);
	if (!surf_grass) {
		spdlog::error("load water pic failed");
		return;
	}

	std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surf_water(IMG_Load("../../assets/pics/water.png"), SDL_DestroySurface);
	if (!surf_water) {
		spdlog::error("load water pic failed");
		return;
	}
	if (!surf_grass || !surf_water) {
		spdlog::error("Texture load failed: %s", SDL_GetError());
	}

	std::shared_ptr<SDL_Texture>tex_grass(SDL_CreateTextureFromSurface(m_renderer.get(), surf_grass.get()), SDL_DestroyTexture);
	std::shared_ptr<SDL_Texture>tex_water(SDL_CreateTextureFromSurface(m_renderer.get(), surf_water.get()), SDL_DestroyTexture);

	// 地图创建
	srand((unsigned int)time(nullptr));
	for (int y = 0; y < height_; y++) {
		for (int x = 0; x < width_; x++) {
			m_world[y][x].tex = (rand() % 5 == 0) ? tex_water : tex_grass;
		}
	}

	for (auto& u : m_units) {
		u.x = (float)(rand() % (width_ * TILE_SIZE));
		u.y = (float)(rand() % (height_ * TILE_SIZE));
	}

}

World::~World()
{
}

void World::run()
{
	float camX = 0, camY = 0;
	bool running = true;
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT) running = false;
		}

		for (auto& u : m_units) {
			u.x += ((rand() % 3) - 1) * 0.5f;
			u.y += ((rand() % 3) - 1) * 0.5f;
		}

		SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 255);
		SDL_RenderClear(m_renderer.get());

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
				SDL_RenderTexture(m_renderer.get(), m_world[y][x].tex.get(), NULL, &r);
			}
		}

		SDL_SetRenderDrawColor(m_renderer.get(), 255, 0, 0, 255);
		for (auto& u : m_units) {
			SDL_FRect r = { u.x - camX, u.y - camY, 8.0f, 8.0f };
			SDL_RenderFillRect(m_renderer.get(), &r);
		}

		SDL_RenderPresent(m_renderer.get());
	}
}
