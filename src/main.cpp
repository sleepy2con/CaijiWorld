#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <cstdlib>
#include <ctime>

const int TILE_SIZE = 16;
const int MAP_W = 300;
const int MAP_H = 300;
const int SCREEN_W = 800;
const int SCREEN_H = 600;

struct Tile {
    SDL_Texture* tex;
};

struct Unit {
    float x, y;
};

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    // 注意：SDL3_image 的 IMG_Init 逻辑与 SDL2 有所不同
    // 如果报错找不到 IMG_INIT_PNG，请检查 SDL_image 版本或直接使用 IMG_Load

    SDL_Window* window = SDL_CreateWindow("300x300 Grid Demo", SCREEN_W, SCREEN_H, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // --- 修复点 1: 移除重复定义，直接加载 ---
    // 建议统一使用 IMG_Load，它可以处理 BMP 和 PNG
    SDL_Surface* surf_grass = IMG_Load("../../assets/pics/grass.png");
    if (!surf_grass) {
        // 如果 PNG 加载失败，尝试加载 BMP（作为备份方案）
        surf_grass = SDL_LoadBMP("../../assets/pics/grass.bmp");
    }

    SDL_Surface* surf_water = IMG_Load("../../assets/pics/water.png");
    if (!surf_water) {
        surf_water = SDL_LoadBMP("../../assets/pics/water.bmp");
    }

    if (!surf_grass || !surf_water) {
        SDL_Log("Texture load failed: %s", SDL_GetError());
    }

    SDL_Texture* tex_grass = SDL_CreateTextureFromSurface(renderer, surf_grass);
    SDL_Texture* tex_water = SDL_CreateTextureFromSurface(renderer, surf_water);

    SDL_DestroySurface(surf_grass);
    SDL_DestroySurface(surf_water);

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

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

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
                SDL_RenderTexture(renderer, map[y][x].tex, NULL, &r);
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (auto& u : units) {
            SDL_FRect r = { u.x - camX, u.y - camY, 8.0f, 8.0f };
            SDL_RenderFillRect(renderer, &r);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(tex_grass);
    SDL_DestroyTexture(tex_water);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}