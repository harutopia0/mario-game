#pragma once
#include <cmath>
#include <cstdlib>
#include <d3d10.h>
#include <d3dx10.h>

class Camera
{
  private:
    static Camera *instance;

    float x, y;
    float screenWidth, screenHeight;
    float mapWidth, mapHeight;
    float smoothSpeed;

    // Shake variables
    float shakeX, shakeY;
    float shakeIntensity;
    float shakeDuration;
    float shakeInitialDuration;

    // Zoom variables
    float zoomScaleFactor;
    float zoomDuration;
    float zoomInitialDuration;
    float currentZoom;

    Camera()
        : x(0), y(0), screenWidth(0), screenHeight(0), mapWidth(0), mapHeight(0), smoothSpeed(0.1f), shakeX(0), shakeY(0), shakeIntensity(0), shakeDuration(0), shakeInitialDuration(0), zoomScaleFactor(1.0f), zoomDuration(0), zoomInitialDuration(0), currentZoom(1.0f)
    {
    }

  public:
    static Camera *GetInstance()
    {
        if (!instance)
            instance = new Camera();
        return instance;
    }

    void Init(float screenW, float screenH, float mapW, float mapH, float smooth = 1.0f)
    {
        screenWidth = screenW;
        screenHeight = screenH;
        mapWidth = mapW;
        mapHeight = mapH;
        smoothSpeed = smooth;
    }

    void SetMapBoundary(float mapW, float mapH)
    {
        mapWidth = mapW;
        mapHeight = mapH;
    }

    void Shake(float intensity, float duration)
    {
        shakeIntensity = intensity;
        shakeDuration = duration;
        shakeInitialDuration = duration;
    }

    void Zoom(float scale, float duration)
    {
        zoomScaleFactor = scale;
        zoomDuration = duration;
        zoomInitialDuration = duration;
    }

    void Update(float targetX, float targetY, float deltaTime)
    {
        x = targetX - 100.0f;
        x = max(0.0f, min(x, mapWidth - screenWidth));

        // Update shake decay
        if (shakeDuration > 0.0f)
        {
            shakeDuration -= deltaTime;
            if (shakeDuration <= 0.0f)
            {
                shakeDuration = 0.0f;
                shakeX = 0.0f;
                shakeY = 0.0f;
            }
            else
            {
                float progress = shakeDuration / shakeInitialDuration;
                float currentIntensity = shakeIntensity * progress;
                shakeX = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * currentIntensity;
                shakeY = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * currentIntensity;
            }
        }

        // Update zoom decay
        if (zoomDuration > 0.0f)
        {
            zoomDuration -= deltaTime;
            if (zoomDuration <= 0.0f)
            {
                zoomDuration = 0.0f;
                currentZoom = 1.0f;
            }
            else
            {
                float progress = zoomDuration / zoomInitialDuration;
                // Sine wave curve: rises to zoomScaleFactor and falls back to 1.0f
                currentZoom = 1.0f + (zoomScaleFactor - 1.0f) * sin(progress * 3.14159265f);
            }
        }
    }

    D3DXMATRIX GetViewMatrix() const
    {
        D3DXMATRIX matTranslate;
        D3DXMatrixTranslation(&matTranslate, -(x + shakeX) * 2.0f, -(y + shakeY) * 2.0f, 0.0f);
        return matTranslate;
    }

    float GetX() const
    {
        return x;
    }
    float GetY() const
    {
        return y;
    }
    float GetMapWidth() const
    {
        return mapWidth;
    }

    float GetShakeX() const
    {
        return shakeX;
    }
    float GetShakeY() const
    {
        return shakeY;
    }
    float GetZoom() const
    {
        return currentZoom;
    }

    float ScreenToWorldX(float screenX) const
    {
        return screenX + x;
    }
    float ScreenToWorldY(float screenY) const
    {
        return screenY + y;
    }

    bool IsVisible(float objX, float objY, float objW, float objH) const
    {
        return objX + objW > x && objX < x + screenWidth &&
               objY + objH > y && objY < y + screenHeight;
    }

    Camera(const Camera &) = delete;
    Camera &operator=(const Camera &) = delete;
};