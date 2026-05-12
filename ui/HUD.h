#pragma once
#include <string>
#include <Windows.h>
#include "../render/Sprites.h"

class HUD
{
private:
    static HUD* __instance;

    int time;
    DWORD timeAccumulator;

    DWORD pMeterBlinkTime;
    bool isPMeterBlinkVisible;

    int currentPMeter;
    int currentPlayer;
    int currentScore;
    int currentCoins;
    int currentLives;
    int currentWorld;

    int cards[3];

    // Vẽ một chuỗi số (text) tại tọa độ x, y chỉ định (Không cần quan tâm hàm này, chỉ là hàm tiện ích để vẽ các số trên HUD)
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

    // Cập nhật các trạng thái của HUD theo thời gian (đếm ngược, nhấp nháy PMeter, v.v.)
    void Update(DWORD dt);

    // Gọi các hàm vẽ để hiển thị toàn bộ HUD lên màn hình
    void Render();

    // Vẽ số lượng đồng xu hiện có
    void DrawCoins(int coins);

    // Vẽ điểm số hiện tại
    void DrawScore(int score);

    // Vẽ thời gian còn lại của màn chơi
    void DrawTime(int time);

    // Vẽ thanh năng lượng (P-Meter) dựa trên mức độ chạy
    void DrawPMeter(int powerLevel);

    // Vẽ biểu tượng người chơi (VD: Mario hoặc Luigi)
    void DrawPlayerIcon(int player);

    // Vẽ tối đa 3 thẻ bài ở cuối màn
    void DrawCards();

    // Vẽ số mạng sống còn lại
    void DrawLives(int lives);

    // Vẽ số thứ tự của World hiện tại
    void DrawWorld(int world);

    // Các hàm cập nhật giá trị hiển thị cho HUD từ bên ngoài
    void SetPMeter(int level) { currentPMeter = level; }
    void SetPlayer(int player) { currentPlayer = player; }
    void SetScore(int score) { currentScore = score; }
    void SetCoins(int coins) { currentCoins = coins; }
    void SetLives(int lives) { currentLives = lives; }
    void SetWorld(int world) { currentWorld = world; }

    // Thêm một thẻ bài mới (Nấm, Hoa, hoặc Sao) vào khe trống tính từ trái qua phải
    void AddCard(int cardType);

    // Xóa sạch toàn bộ 3 thẻ bài
    void ClearCards() {
        cards[0] = 0; cards[1] = 0; cards[2] = 0;
    }

    // Kiểm tra xem đã thu thập đủ 3 thẻ bài chưa
    bool IsFullCards() const {
        return cards[0] != 0 && cards[1] != 0 && cards[2] != 0;
    }
};