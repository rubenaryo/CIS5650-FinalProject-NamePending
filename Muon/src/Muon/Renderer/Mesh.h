/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/10
Description : Mesh stores the vertex,index buffers ready to be drawn by DirectX
----------------------------------------------*/
#ifndef EASEL_MESH_H
#define EASEL_MESH_H

#include "DXCore.h"
#include "Shader.h"

namespace Renderer {
struct Mesh
{
    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
    UINT          IndexCount;
    UINT          Stride;
};

struct Mesh_DX12
{
    ~Mesh_DX12();
    bool Init(void* vertexData, UINT vertexDataSize, UINT vertexStride, void* indexData, UINT indexDataSize, UINT indexCount, DXGI_FORMAT indexFormat);
    bool PopulateBuffers(void* vertexData, UINT vertexDataSize, UINT vertexStride, void* indexData, UINT indexDataSize, UINT indexCount);
    bool Draw(ID3D12GraphicsCommandList* pCommandList);

    ID3D12Resource* VertexBuffer = nullptr;
    ID3D12Resource* IndexBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {0};
    D3D12_INDEX_BUFFER_VIEW IndexBufferView = {0};
    UINT IndexCount = 0;
    UINT Stride = 0;
};

}

#endif