#pragma once
#include <string>
#include <Windows.h>
#include "../render/Sprites.h"

class HUD
{
private:
    static HUD* __instance;

    // PMeter blink animation (chỉ thuộc về UI)
    DWORD pMeterBlinkTime;
    bool isPMeterBlinkVisible;

    int currentPMeter;
    int currentPlayer;

    // Vẽ một chuỗi số (text) tại tọa độ x, y chỉ định
    void DrawString(std::string text, float x, float y);

public:
    // Hủy instance (dọn dẹp bộ nhớ cho pattern Singleton)
    static void DestroyInstance();

    // Khởi tạo các giá trị mặc định cho HUD
    HUD();

    // Lấy instance duy nhất của HUD (Singleton)
    static HUD* GetInstance();

    // Tải toàn bộ sprite dùng cho HUD từ hệ thống
    void LoadSprites();

    // Cập nhật trạng thái UI (nhấp nháy PMeter)
    void Update(DWORD dt);

    // Gọi các hàm vẽ để hiển thị toàn bộ HUD lên màn hình
    void Render();

    // Vẽ số lượng quái vật hạ gục được
    void DrawKills(int kills);

    // Vẽ điểm số hiện tại
    void DrawScore(int score);

    // Vẽ thời gian còn lại của màn chơi
    void DrawTime(int time);

    // Vẽ thanh năng lượng (P-Meter) dựa trên mức độ chạy
    void DrawPMeter(int powerLevel);

    // Vẽ biểu tượng người chơi (VD: Mario hoặc Luigi)
    void DrawPlayerIcon(int player);

    // Vẽ tối đa 3 thẻ bài (đọc từ GameManager)
    void DrawCards();

    // Vẽ số mạng sống còn lại
    void DrawLives(int lives);

    // Vẽ số thứ tự của World hiện tại
    void DrawWorld(int world);

    // Các hàm cập nhật giá trị hiển thị cho HUD từ bên ngoài
    void SetPMeter(int level) { currentPMeter = level; }
    void SetPlayer(int player) { currentPlayer = player; }
};