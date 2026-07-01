// Copyright 2026 He Yuxuan. All Rights Reserved.
#include "caijiworld/render/viewport.h"
#include "caijiworld/constant/constant.h"
#include <algorithm>

namespace caijiworld {

    Viewport::Viewport(int win_width, int win_height)
        : win_width_(win_width), win_height_(win_height) {
    }

    void Viewport::SetZoom(float zoom) {
        zoom_ = std::clamp(zoom, kMinZoom, kMaxZoom);
    }

    void Viewport::AddZoom(float delta) {
        zoom_ += delta;
        zoom_ = std::clamp(zoom_, kMinZoom, kMaxZoom);
    }

    void Viewport::MoveCamera(float dx, float dy) {
        camera_x_ += dx;
        camera_y_ += dy;
    }

    SDL_Point Viewport::ScreenToWorld(float screen_x, float screen_y) const {
        // 先还原屏幕坐标对应的世界像素坐标
        float visible_w = GetVisibleWidth();
        float visible_h = GetVisibleHeight();

        float world_click_x = camera_x_ + (screen_x / static_cast<float>(win_width_)) * visible_w;
        float world_click_y = camera_y_ + (screen_y / static_cast<float>(win_height_)) * visible_h;

        // 再换算为格子坐标（左上角为 0,0）
        return SDL_Point{
            static_cast<int>(world_click_x / kTileSize),
            static_cast<int>(world_click_y / kTileSize)
        };
    }

    SDL_Point Viewport::WorldToScreen(float world_x, float world_y) const {
        // 世界像素坐标 → 屏幕像素坐标
        return SDL_Point{
            static_cast<int>((world_x - camera_x_) * zoom_),
            static_cast<int>((world_y - camera_y_) * zoom_)
        };
    }

} // namespace caijiworld
