#include "dllmain.h"
#include <string>

//https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes windows key codes

uintptr_t clientDllBase = 0;
uintptr_t engineDllBase = 0;

uintptr_t entityList;
Player* localPlayer = nullptr;

bool inGame = false;

int screenWidth = 0;
int screenHeight = 0;

int currentMenuSelection = 0;
const int numberOfSelectables = 3;

bool hideMenu = false;

bool enableAimbot = false;
bool enableEsp = false;
bool hideEspInfo = true;

DWORD WINAPI Thread(LPVOID param)
{
	clientDllBase = (uintptr_t)GetModuleHandle(L"client.dll");
	engineDllBase = (uintptr_t)GetModuleHandle(L"engine.dll");
	if (clientDllBase == 0 || engineDllBase == 0 || !HookEndScene())
	{
		FreeLibraryAndExitThread((HMODULE)param, 0);
		return 0;
	}

	entityList = (clientDllBase + entityListOffset);

	int aimbotTimer = 0;
	bool useAimbot = false;
	Player* aimbotTargetPlayer = nullptr;
	while (!GetAsyncKeyState(VK_INSERT))
	{
		aimbotTimer++;
		
		if (GetAsyncKeyState(VK_F1) & 1) 
		{
			hideMenu = !hideMenu;
		}
		
		if (GetAsyncKeyState(VK_DOWN) & 1)
		{
			currentMenuSelection++;
			if (currentMenuSelection >= numberOfSelectables) { currentMenuSelection = 0; }
		}
		if (GetAsyncKeyState(VK_UP) & 1)
		{
			currentMenuSelection--;
			if (currentMenuSelection < 0) { currentMenuSelection = numberOfSelectables - 1; }
		}

		if (GetAsyncKeyState(VK_RETURN) & 1)
		{
			switch (currentMenuSelection)
			{
			case 0:
				enableAimbot = !enableAimbot;
				break;
			case 1:
				enableEsp = !enableEsp;
				break;
			case 2:
				hideEspInfo = !hideEspInfo;
				break;
			}
		}

		inGame = *(bool*)(clientDllBase + inGameOffset);
		if (!inGame) { continue; }

		localPlayer = *(Player**)(clientDllBase + localPlayerOffset);
		if (!IsValidPlayer(localPlayer)) { continue; }
		
		if (enableAimbot && GetAsyncKeyState(VK_RBUTTON) & 1)
		{
			useAimbot = !useAimbot;
			if (useAimbot) 
			{ 
				aimbotTargetPlayer = GetClosestPlayer(); 
			}
		}

		if(useAimbot && aimbotTimer > 1000)
		{
			if (!IsValidPlayer(aimbotTargetPlayer)) { useAimbot = false; continue; }
			
			Aimbot(aimbotTargetPlayer);
			aimbotTimer = 0;
		}
	}

	UnhookEndScene();

	FreeLibraryAndExitThread((HMODULE)param, 0);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) 
	{ 
		CreateThread(0, 0, Thread, hModule, 0, 0); 
	}
	
	return TRUE;
}

void Draw()
{
	ImGuiIO& io = ImGui::GetIO();
	screenWidth = io.DisplaySize.x;
	screenHeight = io.DisplaySize.y;
	
	if (!hideMenu) 
	{
		ImGui::Begin("GMod Jesso Cheats", nullptr, ImGuiWindowFlags_NoMove);
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(400, 220), ImGuiCond_Always);

		ImGui::Text("Ins - uninject");
		ImGui::Text("F1 - hide this menu");
		ImGui::Text("Up/Down arrow - navigate menu");
		ImGui::Text("Enter - select");
		ImGui::Text("Right click - aimbot player closest to crosshaire");

		ImU32 red = IM_COL32(255, 0, 0, 255);

		if (currentMenuSelection == 0)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, red);
			ImGui::Checkbox("Aimbot", &enableAimbot);
			ImGui::PopStyleColor(1);
		}
		else { ImGui::Checkbox("Aimbot", &enableAimbot); }

		if (currentMenuSelection == 1)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, red);
			ImGui::Checkbox("ESP", &enableEsp);
			ImGui::PopStyleColor(1);
		}
		else { ImGui::Checkbox("ESP", &enableEsp); }

		if (currentMenuSelection == 2)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, red);
			ImGui::Checkbox("Hide ESP info", &hideEspInfo);
			ImGui::PopStyleColor(1);
		}
		else { ImGui::Checkbox("Hide ESP info", &hideEspInfo); }

		//ImGui::SliderFloat("Aimbot strength", &aimbotStrength, 0.01, 10);

		ImGui::Text("----------------");
		ImGui::Text("For daddy Szirth");

		ImGui::End();
	}

	if (enableEsp)
	{
		if (!inGame || !IsValidPlayer(localPlayer)) { return; }

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::Begin("invis window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(screenWidth, screenHeight), ImGuiCond_Always);

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* drawList = window->DrawList;

		ESP(drawList);

		window->DrawList->PushClipRectFullScreen();
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);
	}
}

bool IsValidPlayer(Player* player)
{
	if ((uintptr_t)player < 0x10000) { return false; }
	return player->health > 0;
}

void PredictPosition(Player* targetPlayer, Vector3& out)
{
	if (!IsValidPlayer(localPlayer) || !IsValidPlayer(targetPlayer)) { return; }

	Vector3 velocity = targetPlayer->velocity - localPlayer->velocity;

	out.x += velocity.x / 50;
	out.y += velocity.y / 50;
	out.z += velocity.z / 50;
}

Player* GetClosestPlayer() // by distance to crosshaire
{
	float minDistance = 999999999999.0f;

	Player* targetPlayer = 0;

	for (int i = 0; i < maxPlayerCount; i++)
	{
		Player* player = *(Player**)(entityList + (0x10 * i));
		if (!IsValidPlayer(player) || player == localPlayer) { continue; }

		Vector3 worldDiff = localPlayer->pos - player->pos;
		float distance = sqrt((worldDiff.x * worldDiff.x) + (worldDiff.y * worldDiff.y) + (worldDiff.z * worldDiff.z));
		if (distance > 6000) { continue; }

		Vector2 crosshair = { screenWidth / 2, screenHeight / 2 };
		Vector2 diff = crosshair - GetPlayerScreenPos(player);
		distance = sqrt((diff.x * diff.x) + (diff.y * diff.y));

		if (distance < minDistance)
		{
			minDistance = distance;
			targetPlayer = player;
		}
	}

	return targetPlayer;
}

Vector2 GetPlayerScreenPos(Player* player)
{
	Vector2 result = {};

	Vector3 localPlayerPos = localPlayer->pos;
	localPlayerPos.z += localPlayer->headHeight - maxHeadHeight;

	Vector3 targetPlayerPos = player->pos;
	targetPlayerPos.z += player->headHeight - maxHeadHeight;
	

	PredictPosition(player, targetPlayerPos);

	Vector3 diff = localPlayerPos - targetPlayerPos;
	float distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
	if (distance == 0) { return result; }

	float pitchToPlayer = -(asin((targetPlayerPos.z - localPlayerPos.z) / distance) * rToD);
	float yawToPlayer = (atan2(targetPlayerPos.y - localPlayerPos.y, targetPlayerPos.x - localPlayerPos.x) * rToD);

	float relativePitch = pitchToPlayer - (*(float*)(engineDllBase + viewAnglesOffset));
	float relativeYaw = (*(float*)(engineDllBase + viewAnglesOffset + sizeof(float))) - yawToPlayer;

	if (relativeYaw > 180) { relativeYaw = -(360 - relativeYaw); }
	if (relativeYaw < -180) { relativeYaw = (360 + relativeYaw); }

	// https://www.desmos.com/calculator/xkjavgedgc
	float xFov = (-0.005 * relativeYaw * relativeYaw) + (-0.5 * abs(relativeYaw)) + 125;
	float yFov = (-0.005 * relativePitch * relativePitch) + (-0.15 * abs(relativePitch)) + 70;

	if (xFov == 0 || yFov == 0) { return result; }

	float screenY = relativePitch / (yFov * 0.5);
	screenY = (screenY + 1) / 2;
	screenY *= screenHeight;

	float screenX = relativeYaw / (xFov * 0.5);

	screenX = (screenX + 1) / 2;
	screenX *= screenWidth;

	result.x = screenX;
	result.y = screenY;

	return result;
}

void Aimbot(Player* targetPlayer)
{
	Vector3 localPlayerPos = localPlayer->pos;
	localPlayerPos.z += localPlayer->headHeight - maxHeadHeight;

	Vector3 targetPlayerPos = targetPlayer->pos;
	targetPlayerPos.z -= 25;

	PredictPosition(targetPlayer, targetPlayerPos);
	
	Vector3 diff = localPlayerPos - targetPlayerPos;
	float distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
	if (distance == 0) { return; }

	float pitch = -(asin((targetPlayerPos.z - localPlayerPos.z) / distance) * rToD);
	float yaw = (atan2(targetPlayerPos.y - localPlayerPos.y, targetPlayerPos.x - localPlayerPos.x) * rToD);

	*(float*)(engineDllBase + viewAnglesOffset) = pitch;
	*(float*)(engineDllBase + viewAnglesOffset + sizeof(float)) = yaw;
}

void ESP(ImDrawList* drawList)
{
	if (drawList == nullptr) { return; }

	for (int i = 0; i < maxPlayerCount; i++)
	{
		Player* player = *(Player**)(entityList + (0x10 * i));
		if (!IsValidPlayer(player) || player == localPlayer) { continue; }

		Vector2 screenPos = GetPlayerScreenPos(player);

		Vector3 diff = localPlayer->pos - player->pos;
		float distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
		if (distance == 0) { continue; }

		float depth = (distance * 0.01);
		float sizeX = (screenWidth * 0.05) / depth;
		float sizeY = (screenHeight * 0.425) / depth;

		if ((screenPos.y < -sizeY || screenPos.y > screenHeight) || (screenPos.x < -sizeX || screenPos.x > screenWidth)) { continue; }

		ImU32 color;
		if (player->health > 80) { color = IM_COL32(0, 255, 0, 255); }
		else if (player->health > 30) { color = IM_COL32(255, 255, 0, 255); }
		else { color = IM_COL32(255, 0, 0, 255); }

		if (!hideEspInfo)
		{
			std::string healthStr = "Health: " + std::to_string(player->health);
			drawList->AddText(ImVec2(screenPos.x - sizeX, screenPos.y - 25), color, healthStr.c_str());

			std::string distStr = "Distance: " + std::to_string((int)distance);
			drawList->AddText(ImVec2(screenPos.x - sizeX, screenPos.y - 15), color, distStr.c_str());
		}

		drawList->AddRect(ImVec2(screenPos.x - sizeX, screenPos.y), ImVec2(screenPos.x + sizeX, screenPos.y + sizeY), color);
	}
}