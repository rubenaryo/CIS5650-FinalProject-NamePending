/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/3
Description : Wrapper for Vertex/Pixel/other shader code
----------------------------------------------*/
#ifndef SHADER_H
#define SHADER_H

#include "DXCore.h"
#include "ThrowMacros.h"

#include <vector>
#include <wrl/client.h>

namespace Renderer {

#pragma region VertexShader Stuff
typedef uint8_t semantic_t;
enum class Semantics : semantic_t
{
    POSITION,
    NORMAL,
    TEXCOORD,
    TANGENT,
    BINORMAL,
    COLOR,
    BLENDINDICES,
    BLENDWEIGHTS,
    WORLDMATRIX,
    COUNT
};

struct VertexBufferDescription
{
    Semantics* SemanticsArr = nullptr;
    uint16_t*  ByteOffsets = nullptr;
    uint16_t   AttrCount = 0;
    uint16_t   ByteSize = 0;
};
#pragma endregion

struct VertexShader
{
    ID3D11InputLayout*  InputLayout;
    ID3D11VertexShader* Shader;
    VertexBufferDescription VertexDesc;
    VertexBufferDescription InstanceDesc; // Note: The allocated memory inside this one is contiguous with VertexDesc, so no additional free's are required.
    BOOL Instanced;
};

struct VertexShader_DX12
{
    VertexShader_DX12() = default;
    VertexShader_DX12(const wchar_t* path);
    ~VertexShader_DX12();

    bool Init(const wchar_t* path);

    std::vector<D3D12_INPUT_ELEMENT_DESC> InputElements;
    Microsoft::WRL::ComPtr<ID3DBlob> ShaderBlob;
    VertexBufferDescription VertexDesc;
    VertexBufferDescription InstanceDesc; // Note: The allocated memory inside this one is contiguous with VertexDesc, so no additional free's are required.
    BOOL Initialized = false;
    BOOL Instanced = false;
};

struct PixelShader
{
    ID3D11SamplerState* SamplerState;
    ID3D11PixelShader*  Shader;
};


struct PixelShader_DX12
{
    PixelShader_DX12() = default;
    PixelShader_DX12(const wchar_t* path);
    
    bool Init(const wchar_t* path);

    Microsoft::WRL::ComPtr<ID3DBlob> ShaderBlob;
    BOOL Initialized = false;
};

}

#endif