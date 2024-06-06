#pragma once
#include "memoryTools.h"

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

#include "imgui\imgui.h"
#include "imgui\imgui_impl_win32.h"
#include "imgui\imgui_impl_dx9.h"
#include "ImGui\imgui_internal.h"

typedef HRESULT(APIENTRY* EndSceneType)(LPDIRECT3DDEVICE9 pDevice);

bool HookEndScene();

void UnhookEndScene();

void APIENTRY DetourEndScene(LPDIRECT3DDEVICE9 device);

HWND GetProcessWindow();

void Draw(); // defined in dllmain.cpp