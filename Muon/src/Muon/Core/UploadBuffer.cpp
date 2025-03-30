/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Implementation of GPU Upload Buffers
----------------------------------------------*/
#include "UploadBuffer.h"

#include <d3dx12.h>
#include <Muon/Core/DXCore.h>
#include <Muon/Renderer/ThrowMacros.h>

namespace Muon
{

UploadBuffer::UploadBuffer()
{

}

UploadBuffer::~UploadBuffer()
{
	TryDestroy();
}

void UploadBuffer::Create(const wchar_t* name, size_t size)
{
	if (!GetDevice())
		return;

	TryDestroy();

	mBufferSize = size;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC ResourceDesc = {};
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Width = mBufferSize;
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(mpResource.GetAddressOf()));
	COM_EXCEPT(hr);

	mpResource->SetName(name);
}

void UploadBuffer::TryDestroy()
{
	if (mpResource)
		mpResource->Release();

	mpResource = nullptr;
}

void* UploadBuffer::Map()
{
	void* Memory;
	mpResource->Map(0, &CD3DX12_RANGE(0, mBufferSize), &Memory);
	return Memory;
}

void UploadBuffer::Unmap(size_t begin, size_t end)
{
	mpResource->Unmap(0, &CD3DX12_RANGE(begin, std::min<size_t>(end, mBufferSize)));
}

size_t GetConstantBufferSize(size_t desiredSize) 
{
	return (desiredSize + 255) & ~255;
}

}