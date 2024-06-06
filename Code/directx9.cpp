#include "directx9.h"

HWND window = 0;

void* deviceVTable[119];
BYTE overWrittenBytes[7];
EndSceneType EndSceneGateway = nullptr;

bool HookEndScene()
{
	IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d9 == nullptr) { return false; }

	IDirect3DDevice9* device = nullptr;
	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetProcessWindow();

	HRESULT dummyDevCreated = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device);
	if (dummyDevCreated != S_OK)
	{
		d3dpp.Windowed = !d3dpp.Windowed; // try again in full screen

		HRESULT dummyDevCreated = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device);
		if (dummyDevCreated != S_OK)
		{
			d3d9->Release();
			return false;
		}
	}

	memcpy(deviceVTable, *(void***)(device), sizeof(deviceVTable)); // retrieving vtable
	device->Release();
	d3d9->Release();
	
	memcpy(overWrittenBytes, (char*)deviceVTable[42], 7);

	EndSceneGateway = (EndSceneType)TrampolineHook((BYTE*)deviceVTable[42], (BYTE*)DetourEndScene, 7, false);

	return true;
}

void UnhookEndScene()
{
	SetBytes((BYTE*)deviceVTable[42], overWrittenBytes, sizeof(overWrittenBytes)); // restor original bytes
}

bool init = true;
void APIENTRY DetourEndScene(LPDIRECT3DDEVICE9 device)
{
	if (init) 
	{
		// Init ImGui 
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX9_Init(device);

		init = false;
	}
	
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	Draw();

	ImGui::EndFrame();

	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	EndSceneGateway(device); // resume function execution
}

BOOL CALLBACK enumWindow(HWND handle, LPARAM lp)
{
	DWORD procId;
	GetWindowThreadProcessId(handle, &procId);

	if (GetCurrentProcessId() != procId) { return TRUE; }

	window = handle;

	return FALSE; // stop enumerating
}

HWND GetProcessWindow() // get main window of the game
{
	window = NULL;
	EnumWindows(enumWindow, NULL);

	return window;
}