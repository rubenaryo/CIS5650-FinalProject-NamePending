/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/4
Description : Mesh initialization logic, for DX12
----------------------------------------------*/
#include "Mesh.h"

#include <Muon/Core/DXCore.h>
#include <Muon/Renderer/ThrowMacros.h>
#include <Muon/Renderer/ResourceCodex.h>
#include <Muon/Utils/Utils.h>

namespace Renderer
{

// Creates a vertex/index buffer using the default heap, but does NOT populate it with initial data.
bool CreateBuffer(void* bufferData, UINT bufferDataSize, ID3D12Resource*& out_buffer)
{
    if (!Muon::GetDevice())
        return false;

    // Create VBO in default heap type
    HRESULT hr = Muon::GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferDataSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&out_buffer)
    );
    COM_EXCEPT(hr);

    return out_buffer != nullptr;
}

Mesh_DX12::~Mesh_DX12()
{
    // TODO: I think in DX12 we have to keep track of whether these are still in use by the GPU or not...
    if (IndexBuffer)
        IndexBuffer->Release();
    if (VertexBuffer)
        VertexBuffer->Release();
}

bool Mesh_DX12::Init(void* vertexData, UINT vertexDataSize, UINT vertexStride, void* indexData, UINT indexDataSize, UINT indexCount, DXGI_FORMAT indexFormat)
{
    if (!vertexData || !CreateBuffer(vertexData, vertexDataSize, this->VertexBuffer))
    {
        Muon::Print("Error: Initialized mesh without vertices.\n");
        return false;
    }

    if (!indexData || !CreateBuffer(indexData, indexDataSize, this->IndexBuffer))
    {
        Muon::Print("Info: Initialized mesh without indices.\n");
    }

    if (!PopulateBuffers(vertexData, vertexDataSize, vertexStride, indexData, indexDataSize, indexCount))
    {
        return false;
    }

    // Initialize vertex buffer view
    VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
    VertexBufferView.StrideInBytes = vertexStride;
    VertexBufferView.SizeInBytes = vertexDataSize;

    // Create IBO in default heap type
    if (IndexBuffer)
    {
        IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
        IndexBufferView.Format = indexFormat;
        IndexBufferView.SizeInBytes = indexDataSize;
    }

    IndexCount = indexCount;

    return true;
}

bool Mesh_DX12::PopulateBuffers(void* vertexData, UINT vertexDataSize, UINT vertexStride, void* indexData, UINT indexDataSize, UINT indexCount)
{
    ResourceCodex& codex = ResourceCodex::GetSingleton();
    Muon::UploadBuffer& stagingBuffer = codex.GetStagingBuffer();
    ID3D12GraphicsCommandList* pCommandList = Muon::GetCommandList();

    bool bDoIndexBuffer = indexData && indexDataSize > 0;
    UINT totalRequestedSize = bDoIndexBuffer ? vertexDataSize + indexDataSize : vertexDataSize;

    if (!pCommandList || !stagingBuffer.CanAllocate(totalRequestedSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
        return false;

    void* vboMappedPtr;
    void* iboMappedPtr;
    D3D12_GPU_VIRTUAL_ADDRESS vboGpuAddr, iboGpuAddr;
    UINT vboOffset, iboOffset;

    bool allocateSuccess = stagingBuffer.Allocate(
        vertexDataSize,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
        vboMappedPtr,
        vboGpuAddr,
        vboOffset
    );

    if (!allocateSuccess)
    {
        // This should never happen since we check CanAllocate above...
        return false;
    }

    // Copy the vertex data into the upload buffer
    memcpy(vboMappedPtr, vertexData, vertexDataSize);

    // Schedule a copy from the staging buffer to the real vertex buffer
    pCommandList->CopyBufferRegion(VertexBuffer, 0, stagingBuffer.GetResource(), vboOffset, vertexDataSize);

    if (bDoIndexBuffer)
    {
        allocateSuccess = stagingBuffer.Allocate(
            indexDataSize,
            D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
            iboMappedPtr,
            iboGpuAddr,
            iboOffset
        );

        if (!allocateSuccess)
        {
            return false;
        }

        memcpy(iboMappedPtr, indexData, indexDataSize);
        pCommandList->CopyBufferRegion(IndexBuffer, 0, stagingBuffer.GetResource(), iboOffset, indexDataSize);
    }

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        VertexBuffer,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    ));

    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        IndexBuffer,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_INDEX_BUFFER
    ));

    return true;
}

bool Mesh_DX12::Draw(ID3D12GraphicsCommandList* pCommandList)
{
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
    pCommandList->DrawInstanced(3, 1, 0, 0);

    return true;
}

}