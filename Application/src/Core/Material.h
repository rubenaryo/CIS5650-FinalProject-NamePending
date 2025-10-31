/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/2
Description : Material class for shader information
----------------------------------------------*/
#ifndef MATERIAL_H
#define MATERIAL_H

#include <Core/DXCore.h>
#include "CBufferStructs.h"

#include <Core/PipelineState.h>
#include <Core/Shader.h>
#include <unordered_map>
#include <string>

namespace Muon
{
struct VertexShader;
struct PixelShader;
}

namespace Muon
{

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
    void Destroy();

    const std::string& GetName() const { return mName; }
    void SetVertexShader(const VertexShader* vs);
    void SetPixelShader(const PixelShader* ps);
    //void AddParameter(const char* paramName, ParameterType type);
    
    const ParameterDesc* GetParameter(const char* paramName) const;
    const std::vector<ParameterDesc>& GetAllParameters() const { return mParameters; }
    const std::vector<ConstantBufferReflection>& GetConstantBuffers() const { return mConstantBuffers; }

    bool Generate(DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT);

protected:
    bool MergeShaderResources();
    bool GenerateRootSignature();
    bool GeneratePipelineState(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);

    const VertexShader* mpVS = nullptr;
    const PixelShader* mpPS = nullptr;

    std::vector<ShaderResourceBinding> mResources;
    std::vector<ConstantBufferReflection> mConstantBuffers;
    std::vector<ParameterDesc> mParameters;

    std::unordered_map<const char*, size_t> mParamNameToIndex;
    std::unordered_map<std::string, int> mCBNameToRootIndex;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;

    std::string mName;

    bool mInitialized = false;

    //TextureSlotFlags mFlags = TSF_NONE; // Used for validating that the right textures have been properly specified by the material instance
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