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
};

#endif // !WORLD_H
