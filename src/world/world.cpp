#include "caijiworld/world/world.h"
#include "caijiworld/constant/constant.h"
#include <SDL3_image/SDL_image.h>
#include <cstdlib>
#include <ctime>

World::World(int width, int height)
    : width_(width), height_(height), world_tile_(height, std::vector<Tile>(width)), units_(1)
{
}

void World::InitWorldData(SDL_Renderer* renderer) {
    // 只负责加载图片和生成随机地图数据 
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surf_grass(IMG_Load("../../assets/pics/grass.png"), SDL_DestroySurface);
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surf_water(IMG_Load("../../assets/pics/water.png"), SDL_DestroySurface);

    std::shared_ptr<SDL_Texture> tex_grass(SDL_CreateTextureFromSurface(renderer, surf_grass.get()), SDL_DestroyTexture);
    std::shared_ptr<SDL_Texture> tex_water(SDL_CreateTextureFromSurface(renderer, surf_water.get()), SDL_DestroyTexture);

    srand((unsigned int)time(nullptr));
    for (auto& tile_row : world_tile_) {
        for (auto& tile : tile_row) {
            tile.tex = (rand() % 5 == 0) ? tex_water : tex_grass;
        }
    }

    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surf_chess(IMG_Load("../../assets/pics/chess.png"), SDL_DestroySurface);
    chess_texture_ = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer, surf_chess.get()), SDL_DestroyTexture);

    for (auto& u : units_) {
        u.x = (float)(rand() % (width_ * kTileSize));
        u.y = (float)(rand() % (height_ * kTileSize));
    }
}

void World::Update(float deltaTime) {
    // 纯逻辑：让 NPC 小人们每秒随机走动 
    for (auto& u : units_) {
        u.x += ((rand() % 3) - 1) * 50.0f * deltaTime;
        u.y += ((rand() % 3) - 1) * 50.0f * deltaTime;
    }
}
