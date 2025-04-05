#ifndef FACTORIES_H
#define FACTORIES_H

#include "DXCore.h"
#include "ResourceCodex.h"
#include "Shader.h"

#include <utility>

namespace Renderer {

struct ShaderFactory final
{
    friend class ResourceCodex;

    // Main initialization function: Takes a codex, which then calls the static functions from ShaderFactory to populate its own hashtables
    static void LoadAllShaders(ID3D11Device* device, ResourceCodex& codex);

private: // For VertexShader
    static void CreateVertexShader(const wchar_t* fileName, VertexShader* out_shader, ID3D11Device* device);
    static void CreateVertexShader_DX12(const wchar_t* path, VertexShader_DX12& out_shader);

    static void BuildInputLayout(ID3D11ShaderReflection* pReflection, ID3D10Blob* pBlob, VertexShader* out_shader, ID3D11Device* device);
    static void BuildInputLayout_DX12(ID3D12ShaderReflection* pReflection, ID3DBlob* pBlob, VertexShader_DX12* out_shader);

    static void AssignDXGIFormatsAndByteOffsets(D3D11_INPUT_CLASSIFICATION slotClass, D3D11_SIGNATURE_PARAMETER_DESC* paramDescs, UINT numInputs, D3D11_INPUT_ELEMENT_DESC* out_inputParams, uint16_t* out_byteOffsets, uint16_t* out_byteSize);
    static void AssignDXGIFormatsAndByteOffsets_DX12(D3D12_INPUT_CLASSIFICATION slotClass, D3D12_SIGNATURE_PARAMETER_DESC* paramDescs, UINT numInputs, D3D12_INPUT_ELEMENT_DESC* out_inputParams, uint16_t* out_byteOffsets, uint16_t* out_byteSize);

private: // For PixelShader
    static void CreatePixelShader(const wchar_t* fileName, PixelShader* out_shader, ID3D11Device* device);
};

struct TextureFactory final
{
    typedef std::pair<TextureID, const ResourceBindChord> TexturePair;
    static void LoadAllTextures(ID3D11Device* device, ID3D11DeviceContext* context, ResourceCodex& codex);
};

struct MeshFactory final
{
    static MeshID CreateMesh(const char* fileName, const VertexBufferDescription* vertAttr, Mesh_DX12& out_meshDX12);
    static void LoadAllMeshes();
};

struct MaterialFactory final
{
    static bool CreateAllMaterials(ID3D11Device* device, ResourceCodex& codex);
};

}
#endif