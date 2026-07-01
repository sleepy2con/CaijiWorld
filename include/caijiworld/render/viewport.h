// Copyright 2026 He Yuxuan. All Rights Reserved.
#ifndef CAIJIWORLD_RENDER_VIEWPORT_H_
#define CAIJIWORLD_RENDER_VIEWPORT_H_

#include <SDL3/SDL.h>

namespace caijiworld {

    // SSOT 原则：Viewport 是摄像机位置(camera_x_, camera_y_)和缩放因子(zoom_)的
    // 唯一权威数据源。其他任何组件（InputManager、UIManager）不持有这些数据的副本，
    // 必须通过 Viewport 的公开接口读写。消除了之前 Game / InputState / Viewport
    // 三处各存一份 zoom 的冗余乱象。
    class Viewport {
    public:
        Viewport(int win_width, int win_height);
        ~Viewport() = default;

        Viewport(const Viewport&) = delete;
        Viewport& operator=(const Viewport&) = delete;

        // ---------- 缩放 ----------
        [[nodiscard]] float GetZoom() const { return zoom_; }
        void SetZoom(float zoom);
        void AddZoom(float delta); // 供鼠标滚轮安全地累加缩放

        // ---------- 相机位置 ----------
        [[nodiscard]] float GetCameraX() const { return camera_x_; }
        [[nodiscard]] float GetCameraY() const { return camera_y_; }
        void SetCameraX(float x) { camera_x_ = x; }
        void SetCameraY(float y) { camera_y_ = y; }
        void MoveCamera(float dx, float dy); // 供拖拽平移

        // ---------- 坐标转换 ----------
        // 屏幕像素坐标 → 世界格子坐标（左上角为 0,0）
        [[nodiscard]] SDL_Point ScreenToWorld(float screen_x, float screen_y) const;
        // 世界像素坐标 → 屏幕像素坐标
        [[nodiscard]] SDL_Point WorldToScreen(float world_x, float world_y) const;

        // ---------- 辅助查询 ----------
        [[nodiscard]] float GetVisibleWidth() const { return static_cast<float>(win_width_) / zoom_; }
        [[nodiscard]] float GetVisibleHeight() const { return static_cast<float>(win_height_) / zoom_; }
        [[nodiscard]] int GetWinWidth() const { return win_width_; }
        [[nodiscard]] int GetWinHeight() const { return win_height_; }

        static constexpr float kMinZoom = 0.3f;
        static constexpr float kMaxZoom = 3.0f;

    private:
        int win_width_ = 0;
        int win_height_ = 0;

        // 缩放因子：1.0 = 原始大小，值越大画面越大（地图被放大）
        float zoom_ = 1.0f;

        // 摄像机在世界坐标系中的位置（像素），即当前画面左上角对应的世界坐标
        float camera_x_ = 0.0f;
        float camera_y_ = 0.0f;
    };

} // namespace caijiworld
#endif // CAIJIWORLD_RENDER_VIEWPORT_H_
