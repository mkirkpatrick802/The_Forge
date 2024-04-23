#include "LeaderboardsUIWindow.h"

#include "Client.h"
#include "imgui.h"

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

		ImGui::Text("%d", 0); ImGui::NextColumn();
	}

	ImGui::Columns(1);
}