#include "ScoreManager.h"

#include <algorithm>

#include "LeaderboardsUIWindow.h"

std::unordered_map<uint8, int> ScoreManager::score = std::unordered_map<uint8, int>();

void ScoreManager::SetScore(const uint8 ID, const int value)
{
	score[ID] = value;

	SortScore();
}

void ScoreManager::AddScore(const uint8 ID)
{
	if (auto result = score.insert({ ID, 0 }); !result.second)
		score[ID] += 1;

	SortScore();
}

void ScoreManager::ResetScore(const uint8 ID)
{
	score[ID] = 0;

	SortScore();
}

int ScoreManager::GetScore(const uint8 ID)
{
	return score[ID];
}

void ScoreManager::SortScore()
{
	std::ranges::sort(LeaderboardsUIWindow::players, [](const ClientObject& a, const ClientObject& b) {
		return score[a.playerID] > score[b.playerID];
		});
}
