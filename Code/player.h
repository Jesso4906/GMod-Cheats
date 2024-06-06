#pragma once
#include "mathStructs.h"

const unsigned int inGameOffset = 0x7DAE40; // offset from client.dll

const unsigned int entityListOffset = 0x75AD24; // offset from client.dll
const unsigned int maxPlayerCount = 128;

const unsigned int localPlayerOffset = 0x7D54CC; // offset from client.dll

const unsigned int viewAnglesOffset = 0x50584C; // offset from engine.dll

const float maxHeadHeight = 64;

const unsigned int healthOffset = 0x90;
const unsigned int headHeightOffset = 0xF0;
const unsigned int velocityOffset = 0xF4;
const unsigned int posOffset = 0x260;

struct Player 
{
	char pad1[healthOffset];
	int health;

	char pad2[headHeightOffset - healthOffset - sizeof(health)];
	float headHeight;

	//char pad3[velocityOffset - headHeightOffset - sizeof(headHeight)]; // pad is 0
	Vector3 velocity;

	char pad4[posOffset - velocityOffset - sizeof(velocity)];
	Vector3 pos;
};