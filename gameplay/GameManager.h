#pragma once
class GameManager
{
	private:
		static GameManager* __instance;
		int level;
		bool isGameOver;
		bool isGameWin;
public:
	static GameManager* GetInstance();
	void SetLevel(int level) { this->level = level; }
	int GetLevel() { return level; }
	void SetGameOver(bool isGameOver) { this->isGameOver = isGameOver; }
	bool IsGameOver() { return isGameOver; }
	void SetGameWin(bool isGameWin) { this->isGameWin = isGameWin; }
	bool IsGameWin() { return isGameWin; }
};

