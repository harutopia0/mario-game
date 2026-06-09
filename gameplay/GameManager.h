#pragma once
class GameManager
{
private:
	static GameManager* __instance;
	int level;
	bool isGameOver;
	bool isGameWin;
	bool isLevelClear; // Thêm cờ trạng thái qua màn nhỏ

	bool isDebugMode;         // F3 debug mode: bypass level lock
	bool clearedLevels[6];    // clearedLevels[1..5] = true nếu đã clear màn đó
public:
	static GameManager* GetInstance();
	void SetLevel(int level) { this->level = level; }
	int GetLevel() { return level; }
	void SetGameOver(bool isGameOver) { this->isGameOver = isGameOver; }
	bool IsGameOver() { return isGameOver; }
	void SetGameWin(bool isGameWin) { this->isGameWin = isGameWin; }
	bool IsGameWin() { return isGameWin; }
	void SetLevelClear(bool isLevelClear) { this->isLevelClear = isLevelClear; } // Thêm
	bool IsLevelClear() { return isLevelClear; } // Thêm

	// Debug mode (F3 toggle)
	void SetDebugMode(bool debug) { isDebugMode = debug; }
	bool IsDebugMode() { return isDebugMode; }

	// Level cleared tracking
	void SetLevelCleared(int lvl, bool cleared) { if (lvl >= 1 && lvl <= 5) clearedLevels[lvl] = cleared; }
	bool IsLevelCleared(int lvl) { if (lvl >= 1 && lvl <= 5) return clearedLevels[lvl]; return false; }
	void ResetClearedLevels();
};