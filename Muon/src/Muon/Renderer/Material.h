/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/2
Description : Material class for shader information
----------------------------------------------*/
#ifndef MATERIAL_H
#define MATERIAL_H

#include <Muon/Core/DXCore.h>
#include "CBufferStructs.h"

#include <Muon/Renderer/PipelineState.h>
#include <unordered_map>
#include <string>

namespace Renderer
{
struct VertexShader;
struct PixelShader;
}

namespace Renderer {

enum class TextureSlots : UINT
{
    DIFFUSE   = 0U,
    NORMAL    = 1U,
    SPECULAR  = 2U,
    ROUGHNESS = 3U,
    CUBE      = 4U,
    COUNT
};

enum TextureSlotFlags : UINT
{
    TSF_NONE        = 0,
    TSF_DIFFUSE     = 1 << (UINT)TextureSlots::DIFFUSE,
    TSF_NORMAL      = 1 << (UINT)TextureSlots::NORMAL,
    TSF_SPECULAR    = 1 << (UINT)TextureSlots::SPECULAR,
    TSF_ROUGHNESS   = 1 << (UINT)TextureSlots::ROUGHNESS,
    TSF_CUBE        = 1 << (UINT)TextureSlots::CUBE, 
};

//struct ResourceBindChord
//{
//    ID3D11ShaderResourceView*  SRVs[(UINT)TextureSlots::COUNT];
//};

// Materials own both VS and PS because they must match in the pipeline
//struct Material
//{
//    const VertexShader*         VS = nullptr;
//    const PixelShader*          PS = nullptr;;
//    const ResourceBindChord*    Resources = nullptr;
//    ID3D11RasterizerState*      RasterStateOverride = nullptr;
//    ID3D11DepthStencilState*    DepthStencilStateOverride = nullptr;
//    cbMaterialParams            Description;
//};

// Note: When adding to this enum, make sure to add to the sParamSizes array below in GetParamTypeSize.
enum class ParameterType
{
    Int = 0,
    Float,
    Float2,
    Float3,
    Float4,
    Count,
    Invalid
};

size_t GetParamTypeSize(ParameterType type);

struct ParameterDesc
{
    ParameterDesc(const char* name, ParameterType type);

    std::string Name;
    ParameterType Type = ParameterType::Invalid;
    UINT Index = 0;
    UINT Offset = 0;
};

struct ParameterValue
{
    union {
        int IntValue;
        float FloatValue;
        DirectX::XMFLOAT2 Float2Value;
        DirectX::XMFLOAT3 Float3Value;
        DirectX::XMFLOAT4 Float4Value;
    };
};

// Material types define the required parameters, shaders, and hold the underlying pipeline state.
class MaterialType
{
public:
    MaterialType(const char* name);

    const std::string& GetName() const { return mName; }
    void SetVertexShader(const VertexShader& vs);
    void SetPixelShader(const PixelShader& ps);
    void SetRootSignature(ID3D12RootSignature* pRootSig);
    void AddParameter(const char* paramName, ParameterType type);
    const ParameterDesc* GetParameter(const char* paramName) const;

    bool Generate();

protected:
    const VertexShader* mpVS = nullptr;
    const PixelShader* mpPS = nullptr;
    std::vector<ParameterDesc> mParameters;
    std::unordered_map<const char*, size_t> mParamNameToIndex;
    Muon::GraphicsPipelineState mPipelineState;
    std::string mName;
    size_t mSize = 0;
    TextureSlotFlags mFlags = TSF_NONE; // Used for validating that the right textures have been properly specified by the material instance
};

// MaterialInstances are immutably tied to their parent type at creation. 
// Any parameters set will be validated against this parent type.
class MaterialInstance
{
public:
    MaterialInstance(const char* name, const MaterialType& materialType);
    bool SetParamValue(const char* paramName, ParameterValue value);

protected:
    const MaterialType& mType;
    std::vector<ParameterValue> mParamValues;
    std::string mName;
};
    

}
#endif