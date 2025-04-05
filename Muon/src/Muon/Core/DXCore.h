/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Holds all the central DX12 Data structures
----------------------------------------------*/
#ifndef MUON_DXCORE_H
#define MUON_DXCORE_H

#include <d3dx12.h>
#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

namespace Muon
{
	ID3D12Device* GetDevice();
	ID3D12CommandQueue* GetCommandQueue();
	ID3D12GraphicsCommandList* GetCommandList();
	ID3D12CommandAllocator* GetCommandAllocator();
	ID3D12Fence* GetFence();
	ID3D12RootSignature* GetRootSignature();

	bool ResetCommandList();
	bool CloseCommandList();
	bool PrepareForRender();
	bool FinalizeRender();
	bool ExecuteCommandList();
	bool Present();
	bool WaitForPreviousFrame();

	bool Initialize(HWND hwnd, int width, int height);
}

#endif