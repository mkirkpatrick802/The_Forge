#include "LeaderboardsUIWindow.h"

#include <algorithm>

#include "Client.h"
#include "imgui.h"

#include "ScoreManager.h"

std::vector<ClientObject> LeaderboardsUIWindow::players = std::vector<ClientObject>();

void LeaderboardsUIWindow::Render()
{
	ImGui::Begin("Leaderboards");

	if(players.empty()) { ImGui::Text("No players to display"); return; }

	ImGui::Columns(2, "myColumns");
	ImGui::SetColumnWidth(0, 180.0f);

	ImGui::Text("Nickname"); ImGui::NextColumn();

	ImGui::Text("Score"); ImGui::NextColumn();
	ImGui::Separator();



	for (const auto& player : players)
	{
		ImGui::Text("%s %s", player.nickname.c_str(), (player.playerID == Client::playerID) ? "(You)" : "");
		ImGui::NextColumn();

		ImGui::Text("%d", ScoreManager::GetScore(player.playerID)); ImGui::NextColumn();
	}

	ImGui::Columns(1);
}