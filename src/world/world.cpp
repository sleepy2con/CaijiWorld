
#include "world/world.h" // 1st
#include "constant/constant.h"
#include "world/tile.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
#include "spdlog/spdlog.h" // 3st


#include <SDL3_image/SDL_image.h>

#include <vector>	// 4st
#include <cstdlib>
#include <ctime>
#include <memory>


World::World(int width, int height) :world_tile_(height, std::vector<Tile>(width)), units_(1), width_(width), height_(height),
window_(SDL_CreateWindow("300x300 Grid Demo", width, height, 0), SDL_DestroyWindow),
renderer_(SDL_CreateRenderer(window_.get(), nullptr), SDL_DestroyRenderer)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		spdlog::error("SDL_Init failed: {}", SDL_GetError());
		return;
	}

	// Create window with SDL_Renderer graphics context | 当前操作系统设置的缩放，比如我目前设置了150%，他的值就是1.5，gemini
	float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	const std::string title_str{ "caijiworld_" + std::to_string(width) + "*" + std::to_string(height) };
	// 在函数体内部进行真正的窗口创建,因为它需要先调用 SDL_Init 来初始化 SDL 库，否则会失败。
	window_.reset(SDL_CreateWindow(title_str.c_str(), width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX));
	renderer_.reset(SDL_CreateRenderer(window_.get(), nullptr));

	// 放在这里！告诉渲染器：不管实际窗口多大，我的绘图逻辑永远按 1200*900 来算
	SDL_SetRenderLogicalPresentation(renderer_.get(), width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);

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

	std::shared_ptr<SDL_Texture>tex_grass(SDL_CreateTextureFromSurface(renderer_.get(), surf_grass.get()), SDL_DestroyTexture);
	std::shared_ptr<SDL_Texture>tex_water(SDL_CreateTextureFromSurface(renderer_.get(), surf_water.get()), SDL_DestroyTexture);

	// 设置地图纹理格子
	srand((unsigned int)time(nullptr));
	for (auto& tile_row : world_tile_) {
		for (auto& tile : tile_row) {
			tile.tex = (rand() % 5 == 0) ? tex_water : tex_grass;
		}
	}

	for (auto& u : units_) {
		u.x = (float)(rand() % (width_ * TILE_SIZE));
		u.y = (float)(rand() % (height_ * TILE_SIZE));
	}


	// Setup Dear ImGui context | imgui的上下文设置，初始化等等
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls	允许键盘操作UI
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style | 设置经典主题
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	// 缩放按钮、边框、间距等尺寸,ImGui 默认所有的像素单位都是 1:1。在普通的 1080P 屏幕上看起来正常，但在 4K 笔记本屏幕上，原本 100 像素宽的窗口会缩成指甲盖那么大
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

	//Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLRenderer(window_.get(), renderer_.get());
	ImGui_ImplSDLRenderer3_Init(renderer_.get());
}

World::~World()
{
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}

void World::run()
{
	float camX = 0, camY = 0;
	bool running = true;
	// 游戏主循环
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT) running = false;
			ImGui_ImplSDL3_ProcessEvent(&e);
			if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && e.window.windowID == SDL_GetWindowID(window_.get()))
				running = true;
		}

		for (auto& u : units_) {
			u.x += ((rand() % 3) - 1) * 0.5f;
			u.y += ((rand() % 3) - 1) * 0.5f;
		}

		SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
		SDL_RenderClear(renderer_.get());

		// 视口裁剪渲染 | 只让摄像机范围内的地图格子参与渲染，提升性能
		int startX = (int)(camX / TILE_SIZE);
		int startY = (int)(camY / TILE_SIZE);
		int endX = (int)((camX + kWindowWidth) / TILE_SIZE + 1);
		int endY = (int)((camY + kWindowHeight) / TILE_SIZE + 1);

		// 越界检查
		if (startX < 0) startX = 0;
		if (startY < 0) startY = 0;

		for (int y = startY; y < endY && y < MAP_H; y++) {
			for (int x = startX; x < endX && x < MAP_W; x++) {
				SDL_FRect r = { (float)x * TILE_SIZE - camX, (float)y * TILE_SIZE - camY, (float)TILE_SIZE, (float)TILE_SIZE };
				SDL_RenderTexture(renderer_.get(), world_tile_[y][x].tex.get(), NULL, &r);
			}
		}

		SDL_SetRenderDrawColor(renderer_.get(), 255, 0, 0, 255);
		for (auto& u : units_) {
			SDL_FRect r = { u.x - camX, u.y - camY, 8.0f, 8.0f };
			SDL_RenderFillRect(renderer_.get(), &r);
		}

		SDL_RenderPresent(renderer_.get());
	}
}
