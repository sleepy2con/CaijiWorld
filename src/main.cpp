#include <SDL3/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>

struct Unit
{
    float x;
    float y;
    float vx;
    float vy;
};

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Grid Demo",
        800,
        600,
        0
    );

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, NULL);

    const int gridSize = 20;
    const int mapW = 50;
    const int mapH = 50;

    std::vector<Unit> units;

    srand(time(nullptr));

    // 创建1000个单位
    for (int i = 0; i < 1000; i++)
    {
        Unit u;
        u.x = rand() % (mapW * gridSize);
        u.y = rand() % (mapH * gridSize);
        u.vx = (rand() % 3 - 1) * 0.5f;
        u.vy = (rand() % 3 - 1) * 0.5f;
        units.push_back(u);
    }

    bool running = true;

    while (running)
    {
        SDL_Event e;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
                running = false;
        }

        // 更新单位
        for (auto& u : units)
        {
            u.x += u.vx;
            u.y += u.vy;

            if (u.x < 0 || u.x > mapW * gridSize)
                u.vx = -u.vx;

            if (u.y < 0 || u.y > mapH * gridSize)
                u.vy = -u.vy;
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // 画网格
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);

        for (int x = 0; x <= mapW; x++)
        {
            SDL_RenderLine(
                renderer,
                x * gridSize,
                0,
                x * gridSize,
                mapH * gridSize
            );
        }

        for (int y = 0; y <= mapH; y++)
        {
            SDL_RenderLine(
                renderer,
                0,
                y * gridSize,
                mapW * gridSize,
                y * gridSize
            );
        }

        // 画单位
        SDL_SetRenderDrawColor(renderer, 200, 80, 80, 255);

        for (auto& u : units)
        {
            SDL_FRect r = { u.x, u.y, 6, 6 };
            SDL_RenderFillRect(renderer, &r);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}