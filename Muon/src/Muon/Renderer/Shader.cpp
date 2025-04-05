#include "Shader.h"
/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Implementations of Muon shader objects
----------------------------------------------*/

#include <Muon/Renderer/ThrowMacros.h>

namespace Renderer
{

static const char* kSemanticNames[] =
{
    "POSITION",
    "NORMAL",
    "TEXCOORD",
    "TANGENT",
    "BINORMAL",
    "COLOR",
    "BLENDINDICES",
    "BLENDWEIGHTS",
    "WORLDMATRIX"
};

static const char* kInstancedSemanticNames[] =
{
    "INSTANCE_POSITION",
    "INSTANCE_NORMAL",
    "INSTANCE_TEXCOORD,",
    "INSTANCE_TANGENT",
    "INSTANCE_BINORMAL",
    "INSTANCE_COLOR",
    "INSTANCE_BLENDINDICES",
    "INSTANCE_BLENDWEIGHTS",
    "INSTANCE_WORLDMATRIX"
};

// Previously called AssignDXGIFormatsAndByteOffsets
void PopulateInputElements(D3D12_INPUT_CLASSIFICATION slotClass,
    std::vector < D3D12_SIGNATURE_PARAMETER_DESC> paramDescs,
    UINT numInputs,
    std::vector<D3D12_INPUT_ELEMENT_DESC>& out_inputParams,
    uint16_t* out_byteOffsets,
    uint16_t& out_byteSize)
{
    uint16_t totalByteSize = 0;
    for (uint8_t i = 0; i != numInputs; ++i)
    {
        D3D12_SIGNATURE_PARAMETER_DESC& paramDesc = paramDescs[i];

        out_inputParams.push_back(D3D12_INPUT_ELEMENT_DESC());
        D3D12_INPUT_ELEMENT_DESC& inputParam = out_inputParams.back();
        
        size_t length = 1 +  strnlen_s(paramDesc.SemanticName, 256);
        char* buf = new CHAR[length];
        memset(buf, 0, length);
        strncpy(buf, paramDesc.SemanticName, length);
        inputParam.SemanticName = buf;
        inputParam.SemanticIndex = paramDesc.SemanticIndex;
        inputParam.InputSlotClass = slotClass;

        if (slotClass == D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA)
        {
            inputParam.InputSlot = 0;
            inputParam.InstanceDataStepRate = 0;
        }
        else // per instance data
        {
            inputParam.InputSlot = 1;
            inputParam.InstanceDataStepRate = 1;
        }

        out_byteOffsets[i] = totalByteSize;
        inputParam.AlignedByteOffset = totalByteSize;

        // determine DXGI format ... Thanks MSDN!
        if (paramDesc.Mask == 1) // R
        {
            totalByteSize += 4;
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)   inputParam.Format = DXGI_FORMAT_R32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)   inputParam.Format = DXGI_FORMAT_R32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)   inputParam.Format = DXGI_FORMAT_R32_FLOAT;
        }
        else if (paramDesc.Mask <= 3) // RG
        {
            totalByteSize += 8;
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)   inputParam.Format = DXGI_FORMAT_R32G32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)   inputParam.Format = DXGI_FORMAT_R32G32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)   inputParam.Format = DXGI_FORMAT_R32G32_FLOAT;
        }
        else if (paramDesc.Mask <= 7) // RGB
        {
            totalByteSize += 12;
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)   inputParam.Format = DXGI_FORMAT_R32G32B32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)   inputParam.Format = DXGI_FORMAT_R32G32B32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)   inputParam.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        }
        else if (paramDesc.Mask <= 15) // RGBA
        {
            totalByteSize += 16;
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)   inputParam.Format = DXGI_FORMAT_R32G32B32A32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)   inputParam.Format = DXGI_FORMAT_R32G32B32A32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)   inputParam.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }


        out_inputParams[i] = inputParam;
    }

    out_byteSize = totalByteSize;
}

bool BuildInputLayout(ID3D12ShaderReflection* pReflection, ID3DBlob* pBlob, VertexShader* out_shader)
{
    if (!pReflection || !pBlob || !out_shader)
        return false;

    D3D12_SHADER_DESC shaderDesc;
    pReflection->GetDesc(&shaderDesc);
    const UINT numInputs = shaderDesc.InputParameters;

    Semantics* semanticsArr = new Semantics[numInputs];
    std::vector<D3D12_SIGNATURE_PARAMETER_DESC> paramDescs(numInputs);

    out_shader->Instanced = false;
    ZeroMemory(&out_shader->InstanceDesc, sizeof(VertexBufferDescription));

    uint8_t numInstanceInputs = 0;
    uint8_t instanceStartIdx = UINT8_MAX;

    // First, build semantics enum array from the strings gotten from the reflection
    // If any of the strings are marked as being for instancing, then split off into initialization of two vertex buffers
    const char** comparisonArray = kSemanticNames;
    for ( uint8_t i = 0; i != numInputs; ++i )
    {
        D3D12_SIGNATURE_PARAMETER_DESC* pDesc = &paramDescs[i];

        HRESULT hr = pReflection->GetInputParameterDesc(i, pDesc);
        COM_EXCEPT(hr);

        for (semantic_t s = 0; s != (semantic_t)Semantics::COUNT; ++s)
        {
            // Look for the existence of each HLSL semantic
            LPCSTR& semanticName = pDesc->SemanticName;
            if (!strcmp(semanticName, comparisonArray[s]))
            {
                semanticsArr[i] = (Semantics)s;
                break;
            }
            else if (numInstanceInputs == 0 && strstr(semanticName, "INSTANCE_")) // This is not great, but just look for the special "INSTANCE_" prefix
            {
                out_shader->Instanced = true; // Now we know this is an instanced shader
                numInstanceInputs = numInputs - i;
                instanceStartIdx = i--; // decrement i so we scan this semanticName again
                comparisonArray = kInstancedSemanticNames; // By convention: we are done with regular vertex attributes, switch to comparing with the instanced versions
                break;
            }
        }
    }

    // Now the temp array has all the semantics properly labeled. Use it to create the vertex buffer and the instance buffer(if applicable)
    VertexBufferDescription vbDesc;

    vbDesc.ByteOffsets = new uint16_t[numInputs];//(uint16_t*)malloc(sizeof(uint16_t) * numInputs);
    PopulateInputElements(D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, paramDescs, numInputs, out_shader->InputElements, vbDesc.ByteOffsets, vbDesc.ByteSize);
    vbDesc.SemanticsArr = semanticsArr;
    vbDesc.AttrCount = numInputs;
    out_shader->VertexDesc = vbDesc;
    out_shader->Initialized = true;

    return true;
}

VertexShader::VertexShader(const wchar_t* path)
{
    Init(path);
}

VertexShader::~VertexShader()
{
    delete[] VertexDesc.SemanticsArr;
    delete[] VertexDesc.ByteOffsets;

    for (D3D12_INPUT_ELEMENT_DESC& param : InputElements)
    {
        if (param.SemanticName)
        {
            delete[] param.SemanticName;
            param.SemanticName = nullptr;
        }
    }
}

bool VertexShader::Init(const wchar_t* path)
{
    HRESULT hr = D3DReadFileToBlob(path, this->ShaderBlob.GetAddressOf());
    COM_EXCEPT(hr);

    Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pReflection;
    hr = D3DReflect(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(),
        IID_ID3D12ShaderReflection, (void**)pReflection.GetAddressOf());
    COM_EXCEPT(hr);

    BuildInputLayout(pReflection.Get(), ShaderBlob.Get(), this);
    return SUCCEEDED(hr);
}

PixelShader::PixelShader(const wchar_t* path)
{
    Init(path);
}

bool PixelShader::Init(const wchar_t* path)
{
    HRESULT hr = D3DReadFileToBlob(path, this->ShaderBlob.GetAddressOf());
    COM_EXCEPT(hr);

    Initialized = true;
    return SUCCEEDED(hr);
}

}