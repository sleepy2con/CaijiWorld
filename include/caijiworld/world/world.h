#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <memory>
#include <SDL3/SDL.h> // 仅用于持有纹理指针和渲染矩形，后续甚至可以进一步剥离

// 简化的瓦片和单位结构体
struct Tile {
    std::shared_ptr<SDL_Texture> tex;
    // 以后可以在这里加：int temperature; (温度) | bool has_wall; (是否有墙)
};

struct Unit {
    float x, y;
    // 以后可以在这里加：float health; (血量) | std::string job; (工作)
};

class World {
public:
    World(int width, int height);
    ~World() = default;

    // 初始化世界数据（传入渲染器用来创建和绑定纹理）
    void InitWorldData(SDL_Renderer* renderer);

    // 纯粹的逻辑更新（比如小人乱走），与渲染无关 
    void Update(float deltaTime);

public:
    int width_;
    int height_;
    std::vector<std::vector<Tile>> world_tile_;
    std::vector<Unit> units_;

    // 玩家控制的棋子数据
    std::shared_ptr<SDL_Texture> chess_texture_;
    float chess_x_ = 0.0f;
    float chess_y_ = 0.0f;
};
#endif // !WORLD_H