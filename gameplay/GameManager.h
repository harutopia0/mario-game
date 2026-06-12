#pragma once
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
	int holdingCards[3];      // Lưu trữ tối đa 3 thẻ bài trong GameManager
public:
	static GameManager* GetInstance();
	void SetLevel(int level) { this->level = level; }
	int GetLevel() { return level; }
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

	// Quản lý thẻ bài lưu trữ toàn cục
	void AddCard(int cardType);
	int* GetHoldingCards() { return holdingCards; }
	void ClearHoldingCards();
};