#include "memoryTools.h"
#include "mathStructs.h"
#include "directx9.h"
#include "player.h"

bool IsValidPlayer(Player* player);

void PredictPosition(Player* targetPlayer, Vector3& out);

Player* GetClosestPlayer();

Vector2 GetPlayerScreenPos(Player* player);

void Aimbot(Player* targetPlayer);

void ESP(ImDrawList* drawList);