#pragma once
#include <d3d10.h>
#include <d3dx10.h>

class Camera {
private:
    static Camera* instance;

    float x, y;
    float screenWidth, screenHeight;
    float mapWidth, mapHeight;
    float smoothSpeed;

    Camera()
        : x(0), y(0)
        , screenWidth(0), screenHeight(0)
        , mapWidth(0), mapHeight(0)
        , smoothSpeed(1.0f)
    {
    }

public:
    static Camera* GetInstance() {
        if (!instance)
            instance = new Camera();
        return instance;
    }

    void Init(float screenW, float screenH, float mapW, float mapH, float smooth = 1.0f) {
        screenWidth = screenW;
        screenHeight = screenH;
        mapWidth = mapW;
        mapHeight = mapH;
        smoothSpeed = smooth;
    }

    void Update(float targetX, float targetY, float deltaTime)
    {
        x = targetX - 100.0f;

        x = max(0.0f, min(x, mapWidth - screenWidth));
    }

    // Trả về ma trận dịch chuyển theo camera
    D3DXMATRIX GetViewMatrix() const {
        D3DXMATRIX matTranslate;
        D3DXMatrixTranslation(&matTranslate, -x, -y, 0.0f);
        return matTranslate;
    }

    float GetX() const { return x; }
    float GetY() const { return y; }

    float ScreenToWorldX(float screenX) const { return screenX + x; }
    float ScreenToWorldY(float screenY) const { return screenY + y; }

    bool IsVisible(float objX, float objY, float objW, float objH) const {
        return objX + objW > x && objX < x + screenWidth &&
            objY + objH > y && objY < y + screenHeight;
    }

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
};