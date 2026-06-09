#ifndef TILE_H
#define TILE_H

#include <SDL3/SDL.h>
#include <memory>

struct Tile {
	std::shared_ptr<SDL_Texture> tex{};
};

#endif // !TILE_H
