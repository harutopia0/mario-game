#pragma once
#include <Windows.h>

class GameManager
{
private:
	static GameManager* __instance;
	int level;
	bool isGameOver;
	bool isGameWin;
	bool isLevelClear;

	bool isDebugMode;         // F3 debug mode: bypass level lock
	bool clearedLevels[6];    // clearedLevels[1..5] = true nếu đã clear màn đó
	int holdingCards[3];      // Lưu trữ tối đa 3 thẻ bài

	// Dữ liệu gameplay (nguồn dữ liệu duy nhất)
	int score;
	int kills;
	int lives;
	int time;
	DWORD timeAccumulator;
	float mapRightEdge;

	// Form Mario (giữ qua các màn)
	bool isMarioBig;
	bool isMarioFire;
	bool isMarioScissors;

public:
	static GameManager* GetInstance();
	static void DestroyInstance();

	// Level
	void SetLevel(int level) { this->level = level; }
	int GetLevel() { return level; }
	void SetMapRightEdge(float edge) { this->mapRightEdge = edge; }
	float GetMapRightEdge() { return mapRightEdge; }

	// Game state flags
	void SetGameOver(bool isGameOver) { this->isGameOver = isGameOver; }
	bool IsGameOver() { return isGameOver; }
	void SetGameWin(bool isGameWin) { this->isGameWin = isGameWin; }
	bool IsGameWin() { return isGameWin; }
	void SetLevelClear(bool isLevelClear) { this->isLevelClear = isLevelClear; }
	bool IsLevelClear() { return isLevelClear; }

	// Debug mode (F3 toggle)
	void SetDebugMode(bool debug) { isDebugMode = debug; }
	bool IsDebugMode() { return isDebugMode; }

	// Level cleared tracking
	void SetLevelCleared(int lvl, bool cleared) { if (lvl >= 1 && lvl <= 5) clearedLevels[lvl] = cleared; }
	bool IsLevelCleared(int lvl) { if (lvl >= 1 && lvl <= 5) return clearedLevels[lvl]; return false; }
	void ResetClearedLevels();

	// Quản lý thẻ bài
	bool AddCard(int cardType);
	int* GetHoldingCards() { return holdingCards; }
	void ClearHoldingCards();
	int UseCard(int slot);  // Sử dụng thẻ tại vị trí slot (0, 1, 2), trả về loại thẻ

	// Score
	void SetScore(int s) { score = s; }
	int GetScore() { return score; }
	void AddScore(int s) { score += s; }

	// Kills
	void SetKills(int k) { kills = k; }
	int GetKills() { return kills; }
	void AddKills(int k) { kills += k; }

	// Lives
	void SetLives(int l) { lives = l; }
	int GetLives() { return lives; }

	// Time (đếm ngược)
	void ResetTime();           // Reset về 300s
	int GetTime() { return time; }
	void SetTime(int t) { time = t; }
	void UpdateTime(DWORD dt);  // Đếm ngược mỗi frame

	// Mario Form (giữ qua các màn, reset khi về Intro)
	void SetMarioBig(bool big) { isMarioBig = big; }
	bool IsMarioBig() { return isMarioBig; }
	void SetMarioFire(bool fire) { isMarioFire = fire; }
	bool IsMarioFire() { return isMarioFire; }
	void SetMarioScissors(bool scissors) { isMarioScissors = scissors; }
	bool IsMarioScissors() { return isMarioScissors; }


	// Reset một số giá trị khi vào màn chơi mới
	void ResetForNewLevel();
};