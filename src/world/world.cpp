
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
	float camX = 0.0f, camY = 0.0f;
	bool running = true;

	// 用于计算 Delta Time（两帧之间的时间差），保证不同帧率下运动速度一致
	Uint64 last_time = SDL_GetTicks();

	// 游戏主循环
	while (running) {
		// 1. 处理 Delta Time
		Uint64 current_time = SDL_GetTicks();
		float deltaTime = (current_time - last_time) / 1000.0f; // 秒为单位
		last_time = current_time;

		// 2. 事件处理
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			// 将事件传递给 ImGui 处理（如点击按钮、拖动窗口）
			ImGui_ImplSDL3_ProcessEvent(&e);

			if (e.type == SDL_EVENT_QUIT) {
				running = false;
			}
			if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && e.window.windowID == SDL_GetWindowID(window_.get())) {
				running = false;
			}
		}

		// 3. 启动 ImGui 新帧 (必须在绘制前调用)
		//确保字体纹理已加载到 GPU。
		//准备图形资源。 准备好 SDL_Renderer 所需的绘制状态。
		ImGui_ImplSDLRenderer3_NewFrame(); // (渲染层)
		// 处理输入设备和时间戳。
		//具体干了什么：* 计算两帧之间的时间差（Delta Time），用于动画平滑。
		//    更新鼠标位置、按键状态、游戏手柄输入。
		//    处理窗口大小改变、缩放比例（DPI）等 SDL 窗口事件。
		ImGui_ImplSDL3_NewFrame(); // (后端/输入层)
		ImGui::NewFrame();			// (核心逻辑层)

		// 示例：添加一个简单的 ImGui 调试窗口
		ImGui::Begin("Debug Tools");
		ImGui::Text("Camera Pos: (%.1f, %.1f)", camX, camY);
		// 决定地图视角的变量camX，camY，
		ImGui::SliderFloat("Camera X", &camX, 0, (float)(MAP_W * TILE_SIZE - kWindowWidth));
		ImGui::SliderFloat("Camera Y", &camY, 0, (float)(MAP_H * TILE_SIZE - kWindowHeight));
		ImGui::End();

		// 4. 更新逻辑：移动单位
		// 使用 deltaTime 替代固定步长，确保平滑
		for (auto& u : units_) {
			u.x += ((rand() % 3) - 1) * 50.0f * deltaTime; // 每秒移动约 50 像素
			u.y += ((rand() % 3) - 1) * 50.0f * deltaTime;
		}

		// 5. 渲染准备
		SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
		SDL_RenderClear(renderer_.get());

		// 6. 视口裁剪渲染（Culling）
		// 计算当前摄像机看到的瓦片索引范围
		int startX = std::max(0, (int)(camX / TILE_SIZE));
		int startY = std::max(0, (int)(camY / TILE_SIZE));
		// +2 是为了防止边缘切碎感（多画 1-2 格缓冲区）
		int endX = std::min(MAP_W, (int)((camX + kWindowWidth) / TILE_SIZE) + 2);
		int endY = std::min(MAP_H, (int)((camY + kWindowHeight) / TILE_SIZE) + 2);

		for (int y = startY; y < endY; y++) {
			for (int x = startX; x < endX; x++) {
				// 计算每个格子的屏幕渲染位置：世界坐标 - 摄像机坐标
				SDL_FRect r = {
					(float)x * TILE_SIZE - camX,
					(float)y * TILE_SIZE - camY,
					(float)TILE_SIZE,
					(float)TILE_SIZE
				};
				SDL_RenderTexture(renderer_.get(), world_tile_[y][x].tex.get(), NULL, &r);
			}
		}

		// 7. 渲染单位
		SDL_SetRenderDrawColor(renderer_.get(), 255, 0, 0, 255);
		for (auto& u : units_) {
			// 同样需要进行摄像机偏移处理
			SDL_FRect r = { u.x - camX, u.y - camY, 8.0f, 8.0f };
			// 简单的边界剔除检查：如果单位不在屏幕内则不调用 Draw 函数
			if (r.x + r.w > 0 && r.x < kWindowWidth && r.y + r.h > 0 && r.y < kWindowHeight) {
				SDL_RenderFillRect(renderer_.get(), &r);
			}
		}

		// 8. 渲染 ImGui 面板 (在所有游戏元素之后渲染，保证 UI 在最上层)
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_.get());

		// 9. 提交渲染
		SDL_RenderPresent(renderer_.get());
	}
}
