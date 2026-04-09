#ifndef WORLD_H
#define WORLD_H
#include <world/tile.h>

#include <SDL3/SDL.h>
#include <vector>


struct Unit {
	float x, y;
};

struct Tile;
struct SDL_Renderer;

// 地图类
class World {
public:
	explicit World(int width, int height);
	~World();
	void run();
private:
	// 存放地图网格纹理数据的数组
	std::vector<std::vector<Tile>> world_tile_;
	std::vector<Unit> units_;
	std::unique_ptr <SDL_Window, decltype(&SDL_DestroyWindow)> window_;
	std::unique_ptr <SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer_;
	int width_;
	int height_;

	// 棋子纹理
	std::shared_ptr<SDL_Texture> chess_texture_;
	// 棋子位置（固定在 0,0）
	float chess_x_ = 0.0f;
	float chess_y_ = 0.0f;

	// 控制镜头缩放
	float zoom_level_ = 1.0f; // 1.0 是原大小，2.0 是放大一倍，0.5 是缩小一倍
};

#endif // !WORLD_H
