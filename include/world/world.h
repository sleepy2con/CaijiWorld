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

// µÿÕº¿‡
class World {
public:
	explicit World(int width, int height);
	~World();
	void run();
private:
	std::vector<std::vector<Tile>> m_world;
	std::vector<Unit> m_units;
	std::unique_ptr <SDL_Window, decltype(&SDL_DestroyWindow)> m_window;
	std::unique_ptr <SDL_Renderer, decltype(&SDL_DestroyRenderer)> m_renderer;
};

#endif // !WORLD_H
