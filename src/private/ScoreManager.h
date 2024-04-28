#pragma once
#include <unordered_map>

#include "GameData.h"

class ScoreManager
{
public:

	static void SetScore(uint8 ID, int value);
	static void AddScore(uint8 ID);
	static void ResetScore(uint8 ID);
	static int  GetScore(uint8 ID);
	static void SortScore();

private:

	static std::unordered_map<uint8, int> score;
};
