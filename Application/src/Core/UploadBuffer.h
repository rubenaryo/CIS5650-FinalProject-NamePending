/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Interface for GPU Upload Buffers
----------------------------------------------*/
#ifndef MUON_UPLOADBUFFER_H
#define MUON_UPLOADBUFFER_H

#include <wrl/client.h>
#include <d3d12.h>
#include <stdint.h>
#include <string>

namespace Muon
{

struct UploadBuffer
{
    UploadBuffer();
    ~UploadBuffer();

    void Create(const wchar_t* name, size_t size);
    void TryDestroy();

    void* Map();
    void Unmap(size_t begin, size_t end);

    bool CanAllocate(UINT desiredSize, UINT alignment);
    bool Allocate(UINT desiredSize, UINT alignment, void*& out_mappedPtr, D3D12_GPU_VIRTUAL_ADDRESS& out_gpuAddr, UINT& out_offset);

    size_t GetBufferSize() const { return mBufferSize; }
    ID3D12Resource* GetResource() { return mpResource.Get(); }

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
    {
        return mpResource->GetGPUVirtualAddress();
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mpResource;
    UINT8* mMappedPtr = nullptr;
    std::wstring mName;
    size_t mBufferSize = 0;
    size_t mOffset = 0; // The current offset into the buffer where allocations take place
};

size_t GetConstantBufferSize(size_t desiredSize);

}
#endif