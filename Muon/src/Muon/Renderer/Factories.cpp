#include "Factories.h"

#include "hash_util.h"

// MeshFactory
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// ShaderFactory
#include "Shader.h"

// TextureFactory
#include "Material.h"
#include <filesystem>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

#include <Muon/Utils/Utils.h>
#include <unordered_map>

namespace Renderer {

MeshID MeshFactory::CreateMesh(const char* fileName, const VertexBufferDescription* vertAttr, Mesh_DX12& out_meshDX12)
{
    Assimp::Importer Importer;
    MeshID meshId = fnv1a(fileName);

    // Load assimpScene with proper flags
    const aiScene* pScene = Importer.ReadFile(
        Core::GetModelPathFromFile(fileName),
        aiProcess_Triangulate           |
        aiProcess_JoinIdenticalVertices |   // Remove unnecessary duplicate information
        aiProcess_GenNormals            |   // Ensure normals are generated
        aiProcess_CalcTangentSpace          // Needed for normal mapping
    );

    const VertexBufferDescription vertDesc = *vertAttr;

    if (pScene)
    {
        using namespace DirectX;

        // aiScenes may be composed of multiple submeshes,
        // we want to coagulate this into a single vertex/index buffer
        for (unsigned int i = 0; i != pScene->mNumMeshes; ++i)
        {
            const aiMesh* pMesh = pScene->mMeshes[i];
            const aiVector3D c_Zero(0.0f, 0.0f, 0.0f);

            const UINT numVertices = pMesh->mNumVertices;
            BYTE* vertices = (BYTE*)malloc(vertDesc.ByteSize * numVertices);

            const unsigned int numIndices = pMesh->mNumFaces * 3;
            uint32_t* indices  = (uint32_t*)malloc(sizeof(uint32_t) * numIndices);

            // Process Vertices for this mesh
            for (unsigned int j = 0; j != pMesh->mNumVertices; ++j)
            {
                // Assign needed vertex attributes
                for (unsigned int k = 0; k != vertDesc.AttrCount; ++k)
                {
                    const unsigned int currByteOffset = vertDesc.ByteOffsets[k];
                    const unsigned int nextByteOffset = (k+1) != vertDesc.AttrCount ? vertDesc.ByteOffsets[k+1] : vertDesc.ByteSize;
                    const unsigned int numComponents = (nextByteOffset - currByteOffset) / sizeof(float);
                    BYTE* copyLocation = (vertices + j*vertDesc.ByteSize) + currByteOffset;

                    switch (vertDesc.SemanticsArr[k])
                    {
                        case Semantics::POSITION:
                            assert(pMesh->HasPositions());
                            memcpy(copyLocation, &(pMesh->mVertices[j]), sizeof(float) * numComponents);
                            break;
                        case Semantics::NORMAL:
                            assert(pMesh->HasNormals());
                            memcpy(copyLocation, &(pMesh->mNormals[j]), sizeof(float) * numComponents);
                            break;
                        case Semantics::TEXCOORD:
                            assert(pMesh->HasTextureCoords(0));
                            memcpy(copyLocation, &(pMesh->mTextureCoords[0][j]), sizeof(float) * numComponents);
                            break;
                        case Semantics::TANGENT:
                            assert(pMesh->HasTangentsAndBitangents());
                            memcpy(copyLocation, &(pMesh->mTangents[j]), sizeof(float) * numComponents);
                            break;
                        case Semantics::BINORMAL:
                            assert(pMesh->HasTangentsAndBitangents());
                            memcpy(copyLocation, &(pMesh->mBitangents[j]), sizeof(float) * numComponents);
                            break;
                        case Semantics::COLOR: // Lacks testing
                            assert(pMesh->HasVertexColors(0));
                            memcpy(copyLocation, &(pMesh->mColors[0][j]), sizeof(float) * numComponents);
                            break;
                        #if defined(MN_DEBUG)
                        default:
                            OutputDebugStringA("INFO: Unhandled Vertex Shader Input Semantic when parsing Mesh vertices\n");
                        #endif
                    }
                }
            }

            // Process Indices next
            for (unsigned int j = 0, ind = 0; j < pMesh->mNumFaces; ++j)
            {
                const aiFace& face = pMesh->mFaces[j];

                #if defined(MN_DEBUG)
                    assert(face.mNumIndices == 3); // Sanity check
                #endif

                // All the indices of this face are valid, add to list
                indices[ind++] = face.mIndices[0];
                indices[ind++] = face.mIndices[1];
                indices[ind++] = face.mIndices[2];
            }
            
#if 0
            Mesh tempMesh;

            // Populate Mesh's DX objects
            D3D11_BUFFER_DESC vbd;
            vbd.Usage = D3D11_USAGE_IMMUTABLE;
            vbd.ByteWidth = vertDesc.ByteSize * numVertices; // Number of vertices
            vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vbd.CPUAccessFlags = 0;
            vbd.MiscFlags = 0;
            vbd.StructureByteStride = 0;
            D3D11_SUBRESOURCE_DATA initialVertexData;
            initialVertexData.pSysMem = vertices;
            HRESULT hr = pDevice->CreateBuffer(&vbd, &initialVertexData, &tempMesh.VertexBuffer);

            #if defined(MN_DEBUG)
                COM_EXCEPT(hr);
            #endif

            D3D11_BUFFER_DESC ibd;
            ibd.Usage = D3D11_USAGE_IMMUTABLE;
            ibd.ByteWidth = sizeof(unsigned int) * numIndices; // Number of vertices
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            ibd.CPUAccessFlags = 0;
            ibd.MiscFlags = 0;
            ibd.StructureByteStride = 0;
            D3D11_SUBRESOURCE_DATA initialIndexData;
            initialIndexData.pSysMem = indices;
            hr = pDevice->CreateBuffer(&ibd, &initialIndexData, &tempMesh.IndexBuffer);
            tempMesh.IndexCount = numIndices;
            tempMesh.Stride = vertAttr->ByteSize;

            #if defined(MN_DEBUG)
                COM_EXCEPT(hr);
            #endif

            *out_mesh = tempMesh;
#else
            bool success = out_meshDX12.Init(reinterpret_cast<void*>(vertices), vertDesc.ByteSize * numVertices, vertDesc.ByteSize, reinterpret_cast<void*>(indices), sizeof(unsigned int) * numIndices, numIndices, DXGI_FORMAT_R32_UINT);
            if (!success)
                Muon::Print("Failed to init mesh!\n");
#endif
            free(vertices);
            free(indices);
        }
    }
    #if defined(MN_DEBUG)
    else
    {
        char buf[256];
        sprintf_s(buf, "Error parsing '%s': '%s'\n", fileName, Importer.GetErrorString());
        throw std::exception(buf);
        return 0;
    }
    
    const char vbDebug[] = "_VertexBuffer";
    const char ibDebug[] = "_IndexBuffer";
    char vbName[64];
    char ibName[64];
    ZeroMemory(vbName, 64);
    ZeroMemory(ibName, 64);
    strcat_s(vbName, fileName);
    strcat_s(ibName, fileName);    
    strcat_s(vbName, vbDebug);
    strcat_s(ibName, ibDebug);
    strcat_s(vbName, "\0");
    strcat_s(ibName, "\0");
    
    //HRESULT hr = out_mesh->VertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(vbName), vbName);
    //COM_EXCEPT(hr);
    //hr = out_mesh->IndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(ibName), ibName);
    //COM_EXCEPT(hr);
    #endif
    return meshId;
}

void MeshFactory::LoadAllMeshes()
{
}

void ShaderFactory::LoadAllShaders(ID3D11Device* device, ResourceCodex& codex)
{
    namespace fs = std::filesystem;
    std::string shaderPath = SHADERPATH;

    #if defined(MN_DEBUG)
    if(!fs::exists(shaderPath))
        throw std::exception("Shaders folder doesn't exist!");
    #endif

    // Iterate through folder and load shaders
    for (const auto& entry : fs::directory_iterator(shaderPath))
    {
        std::wstring path = entry.path();
        std::wstring name = entry.path().filename();

        ShaderID hash = fnv1a(name.c_str());

        // Parse file name to decide how to create this resource
        if (name.find(L"VS") != std::wstring::npos)
        {
            codex.AddVertexShader(hash, path.c_str(), device);
        }
        else if (name.find(L"PS") != std::wstring::npos)
        {
            codex.AddPixelShader(hash, path.c_str(), device);
        }
    }
}

void ShaderFactory::CreateVertexShader(const wchar_t* path, VertexShader* out_shader, ID3D11Device* device)
{
    HRESULT hr = E_FAIL;

    // Read bytecode from file to blob
    ID3D10Blob* pBlob;
    hr = D3DReadFileToBlob(path, &pBlob);

    COM_EXCEPT(hr);

    // Creating the actual vertex shader representation from the blob's bytecode:
    hr = device->CreateVertexShader(pBlob->GetBufferPointer(), 
    pBlob->GetBufferSize(), 
    nullptr, 
    &out_shader->Shader);

    COM_EXCEPT(hr);
    
    #if defined(MN_DEBUG)
        const char debugShaderName[] = "VS_Shader";
        hr = out_shader->Shader->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(debugShaderName) - 1, debugShaderName);
        COM_EXCEPT(hr);
    #endif

    // Use the Blob to create a shader reflection
    ID3D11ShaderReflection* pReflection = nullptr;
    hr = D3DReflect(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&pReflection));
    COM_EXCEPT(hr);

    // Use reflecion to build an input layout, finalizing vertex shader populate.
    BuildInputLayout(pReflection, pBlob, out_shader, device);
}

void ShaderFactory::CreatePixelShader(const wchar_t* path, PixelShader* out_shader, ID3D11Device* device)
{
    HRESULT hr = E_FAIL;

    // Read bytecode from file to blob
    ID3D10Blob* pBlob;
    hr = D3DReadFileToBlob(path, &pBlob);

    COM_EXCEPT(hr);

    // Creating the actual vertex shader representation from the blob's bytecode:
    hr = device->CreatePixelShader(pBlob->GetBufferPointer(), 
    pBlob->GetBufferSize(), 
    nullptr, 
    &out_shader->Shader);

    COM_EXCEPT(hr);

    #if defined(MN_DEBUG)
        const char debugShaderName[] = "PS_Shader";
        hr = out_shader->Shader->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(debugShaderName) - 1, debugShaderName);
        COM_EXCEPT(hr);
    #endif

    D3D11_SAMPLER_DESC samplerDesc;
    SecureZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.MaxLOD   = D3D11_FLOAT32_MAX;
    
    hr = device->CreateSamplerState(&samplerDesc, &out_shader->SamplerState);
    COM_EXCEPT(hr);
}

const char* kSemanticNames[] = 
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

const char* kInstancedSemanticNames[] = 
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

void ShaderFactory::BuildInputLayout(ID3D11ShaderReflection* pReflection, ID3D10Blob* pBlob, VertexShader* out_shader, ID3D11Device* device)
{
    // Get a shader description
    D3D11_SHADER_DESC shaderDesc;
    pReflection->GetDesc(&shaderDesc);
    const UINT numInputs = shaderDesc.InputParameters;

    Semantics* tempSemanticsArr = (Semantics*)malloc(sizeof(semantic_t) * numInputs);
    D3D11_SIGNATURE_PARAMETER_DESC* paramDescs = (D3D11_SIGNATURE_PARAMETER_DESC*)malloc(sizeof(D3D11_SIGNATURE_PARAMETER_DESC) * numInputs);

    // Instance fields false by default
    out_shader->Instanced = false;
    ZeroMemory(&out_shader->InstanceDesc, sizeof(VertexBufferDescription));

    uint8_t numInstanceInputs = 0;
    uint8_t instanceStartIdx = UINT8_MAX;

    // Always start with regular vertex semantics
    const char** comparisonArray = kSemanticNames;

    // First, build semantics enum array from the strings gotten from the reflection
    // If any of the strings are marked as being for instancing, then split off into initialization of two vertex buffers
    for (uint8_t i = 0; i != numInputs; ++i)
    {
        pReflection->GetInputParameterDesc(i, &paramDescs[i]);

        // determine semantic and assign easy enum ID
        for (semantic_t s = 0; s != (semantic_t)Semantics::COUNT; ++s)
        {
            if (!strcmp(paramDescs[i].SemanticName, comparisonArray[s]))
            {
                tempSemanticsArr[i] = (Semantics)s;
                break;
            }
            else if (numInstanceInputs == 0 && strstr(paramDescs[i].SemanticName, "INSTANCE_")) // Look for the instanced prefix set in HLSL
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

    D3D11_INPUT_ELEMENT_DESC* allInputParams = (D3D11_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D11_INPUT_ELEMENT_DESC) * numInputs);
    if (numInstanceInputs) // Also need to create an instance buffer layout
    {
        // Regular vertex buffer
        UINT numVertexInputs = numInputs - numInstanceInputs;
        vbDesc.ByteOffsets = (uint16_t*)malloc(sizeof(uint16_t) * numInputs);
        AssignDXGIFormatsAndByteOffsets(D3D11_INPUT_PER_VERTEX_DATA, &paramDescs[0], numVertexInputs, &allInputParams[0], vbDesc.ByteOffsets, &vbDesc.ByteSize);
        vbDesc.SemanticsArr = tempSemanticsArr;
        vbDesc.AttrCount = numVertexInputs;
        out_shader->VertexDesc = vbDesc;
        
        // Instance buffer
        VertexBufferDescription instDesc;
        instDesc.ByteOffsets = &vbDesc.ByteOffsets[instanceStartIdx];
        AssignDXGIFormatsAndByteOffsets(D3D11_INPUT_PER_INSTANCE_DATA, &paramDescs[instanceStartIdx], numInstanceInputs, &allInputParams[instanceStartIdx], instDesc.ByteOffsets, &instDesc.ByteSize);
        instDesc.SemanticsArr = &tempSemanticsArr[instanceStartIdx];
        instDesc.AttrCount = numInstanceInputs;
        out_shader->InstanceDesc = instDesc;
    }
    else // Just regular vertex buffer
    {
        vbDesc.ByteOffsets = (uint16_t*)malloc(sizeof(uint16_t) * numInputs);
        AssignDXGIFormatsAndByteOffsets(D3D11_INPUT_PER_VERTEX_DATA, paramDescs, numInputs, allInputParams, vbDesc.ByteOffsets, &vbDesc.ByteSize);
        vbDesc.SemanticsArr = tempSemanticsArr;
        vbDesc.AttrCount = numInputs;
        out_shader->VertexDesc = vbDesc;
    }

    // Finally, try to create the input layout
    HRESULT hr = device->CreateInputLayout(&allInputParams[0], numInputs, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &out_shader->InputLayout);

    #if defined(MN_DEBUG)
    COM_EXCEPT(hr);
    const char debugNameIL[] = "VS_InputLayout";
    hr = out_shader->InputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(debugNameIL) - 1, debugNameIL);
    COM_EXCEPT(hr);
    #endif

    // Cleanup mallocs
    free(allInputParams);
    free(paramDescs);
}

void ShaderFactory::AssignDXGIFormatsAndByteOffsets(D3D11_INPUT_CLASSIFICATION slotClass, D3D11_SIGNATURE_PARAMETER_DESC* paramDescs, UINT numInputs, D3D11_INPUT_ELEMENT_DESC* out_inputParams, uint16_t* out_byteOffsets, uint16_t* out_byteSize)
{
    uint16_t totalByteSize = 0;
    for (uint8_t i = 0; i != numInputs; ++i)
    {
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc = paramDescs[i];

        D3D11_INPUT_ELEMENT_DESC inputParam = {};
        inputParam.SemanticName = paramDesc.SemanticName;
        inputParam.SemanticIndex = paramDesc.SemanticIndex;
        inputParam.InputSlotClass = slotClass;

        if(slotClass == D3D11_INPUT_PER_VERTEX_DATA)
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
        if ( paramDesc.Mask == 1 ) // R
        {
            totalByteSize += 4;
            if      ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32  )   inputParam.Format = DXGI_FORMAT_R32_UINT;
            else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32  )   inputParam.Format = DXGI_FORMAT_R32_SINT;
            else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 )   inputParam.Format = DXGI_FORMAT_R32_FLOAT;
        }
        else if ( paramDesc.Mask <= 3 ) // RG
        {
            totalByteSize += 8;
            if      ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32  )   inputParam.Format = DXGI_FORMAT_R32G32_UINT;
            else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32  )   inputParam.Format = DXGI_FORMAT_R32G32_SINT;
            else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 )   inputParam.Format = DXGI_FORMAT_R32G32_FLOAT;
        }
        else if ( paramDesc.Mask <= 7 ) // RGB
        {
            totalByteSize += 12;
            if      ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32  )   inputParam.Format = DXGI_FORMAT_R32G32B32_UINT;
            else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32  )   inputParam.Format = DXGI_FORMAT_R32G32B32_SINT;
            else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 )   inputParam.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        }
        else if ( paramDesc.Mask <= 15 ) // RGBA
        {
            totalByteSize += 16;
            if      ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32  )   inputParam.Format = DXGI_FORMAT_R32G32B32A32_UINT;
            else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32  )   inputParam.Format = DXGI_FORMAT_R32G32B32A32_SINT;
            else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 )   inputParam.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }


        out_inputParams[i] = inputParam;
    }

    *out_byteSize = totalByteSize;
}

// Loads all the textures from the directory and returns them as out params to the ResourceCodex
void TextureFactory::LoadAllTextures(ID3D11Device* device, ID3D11DeviceContext* context, ResourceCodex& codex)
{
    namespace fs = std::filesystem;
    std::string texturePath = TEXTUREPATH;

    #if defined(MN_DEBUG)
    if(!fs::exists(texturePath))
        throw std::exception("Textures folder doesn't exist!");
    #endif
    
    std::unordered_map<TextureID, ResourceBindChord> tempTexMap;

    // Iterate through folder and initialize materials
    for (const auto& entry : fs::directory_iterator(texturePath))
    {
        std::wstring path = entry.path().c_str();
        std::wstring name = entry.path().filename().c_str();

        ID3D11ShaderResourceView* pSRV;

        // Parse file name to decide how to create this resource
        size_t pos = name.find(L'_');
        const std::wstring TexName = name.substr(0, pos++);
        const std::wstring TexType = name.substr(pos, 1);
        
        // Parse file extension
        pos = name.find(L'.') + 1;
        const std::wstring TexExt  = name.substr(pos);
        
        HRESULT hr = E_FAIL;

        ID3D11Resource* dummy = nullptr;

        // Special Case: DDS Files (Cube maps with no mipmaps)
        if (TexExt == L"dds")
        {
            hr = DirectX::CreateDDSTextureFromFile(
                device,
                path.c_str(),
                &dummy,
                &pSRV);
        } 
        else // For most textures, use WIC with mipmaps
        {
            hr = DirectX::CreateWICTextureFromFile(
                device, context,    // Passing in the context auto generates mipmaps
                path.c_str(),
                &dummy,
                &pSRV);
            
        }
        // Clean up Texture2D
        dummy->Release();
        assert(!FAILED(hr));

        // Classify based on Letter following '_'
        UINT slot;
        switch (TexType[0]) // This is the character that precedes the underscore in the naming convention
        {
            case 'N': // This is a normal map
                slot = (UINT)TextureSlots::NORMAL;
                break;
            case 'T': // This is a texture
                slot = (UINT)TextureSlots::DIFFUSE;
                break;
            case 'R': // Roughness map
                slot = (UINT)TextureSlots::ROUGHNESS;
                break;
            case 'C': // Cube map
                slot = (UINT)TextureSlots::CUBE;
                break;
            default:
                #if defined(MN_DEBUG)
                    std::wstring debugMsg = L"INFO: Attempted to load a texture with an unrecognized type: ";
                    debugMsg.append(name.c_str());
                    OutputDebugStringW(debugMsg.append(L"\n").c_str());
                #endif

                pSRV->Release(); // The SRV is still created, so it must be released
                pSRV = nullptr;
                continue;
        }

        #if defined(MN_DEBUG)
        if (pSRV)
        {
            size_t byteSize;
            char texDebugName[64];
            wcstombs_s(&byteSize, texDebugName, name.c_str(), name.size());
            hr = pSRV->SetPrivateData(WKPDID_D3DDebugObjectName, byteSize, texDebugName);
            COM_EXCEPT(hr);
        }
        #endif

        TextureID tid = fnv1a(TexName.c_str());
        codex.InsertTexture(tid, slot, pSRV);
    }
}

bool MaterialFactory::CreateAllMaterials(ID3D11Device* device, ResourceCodex& codex)
{
    const uint32_t kLunarId = fnv1a(L"Lunar");       // FNV1A of L"Lunar"

    const ShaderID kInstancedPhongVSID = 0xc8a366aa; // FNV1A of L"InstancedPhongVS.cso"
    const ShaderID kPhongPSID = 0x4dc6e249;          // FNV1A of L"PhongPS.cso"
    const ShaderID kPhongPSNormalMapID = fnv1a(L"Phong_NormalMapPS.cso");
    const ShaderID kWireFramePSID = fnv1a(L"WireframePS.cso");
    const ShaderID kSkyVSID = 0xeb5accd4;         // fnv1a L"SkyVS.cso"
    const ShaderID kSkyPSID = 0x6ec235e6;         // fnv1a L"SkyPS.cso"
    const TextureID kSkyTextureID = 0x2fb626d6;   // fnv1a L"Sky"
    const TextureID kSpaceTextureID = 0xc1c43225; // fnv1a L"Space"
    const MeshID kSkyMeshID = 0x4a986f37; // cube

    {
        Material lunarMaterial;
        lunarMaterial.VS = codex.GetVertexShader(kInstancedPhongVSID);
        lunarMaterial.PS = codex.GetPixelShader(kPhongPSNormalMapID);
        lunarMaterial.Description.colorTint = DirectX::XMFLOAT4(DirectX::Colors::White);
        lunarMaterial.Description.specularExp = 128.0f;
        lunarMaterial.Resources = codex.GetTexture(kLunarId);

        MaterialIndex lunarMaterialIndex = codex.PushMaterial(lunarMaterial);
        assert(MI_LUNAR == lunarMaterialIndex); // This is stupid
    }

    {
        Material skyMaterial;
        skyMaterial.VS = codex.GetVertexShader(kSkyVSID);
        skyMaterial.PS = codex.GetPixelShader(kSkyPSID);
        skyMaterial.Resources = codex.GetTexture(kSpaceTextureID);

        // Back-facing rasterizer state
        D3D11_RASTERIZER_DESC rastDesc = {};
        rastDesc.FillMode = D3D11_FILL_SOLID;
        rastDesc.CullMode = D3D11_CULL_FRONT;
        rastDesc.DepthClipEnable = true;
        device->CreateRasterizerState(&rastDesc, &skyMaterial.RasterStateOverride);

        // Depth stencil with "less equal"
        D3D11_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        device->CreateDepthStencilState(&dsDesc, &skyMaterial.DepthStencilStateOverride);

#if defined(MN_DEBUG)
        const char RSdebugName[] = "Sky_RS";
        const char DSdebugName[] = "Sky_DS";
        HRESULT hr = skyMaterial.RasterStateOverride->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(RSdebugName) - 1, RSdebugName);
        hr = skyMaterial.DepthStencilStateOverride->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(DSdebugName) - 1, DSdebugName);
        COM_EXCEPT(hr);
#endif

        MaterialIndex skyMaterialIndex = codex.PushMaterial(skyMaterial);
        assert(MI_SKY == skyMaterialIndex); // This is stupid    
    }

    {
        Material wireframeMaterial;
        wireframeMaterial.VS = codex.GetVertexShader(kInstancedPhongVSID);
        wireframeMaterial.PS = codex.GetPixelShader(kWireFramePSID);
        wireframeMaterial.Description.colorTint = DirectX::XMFLOAT4(DirectX::Colors::White);
        wireframeMaterial.Description.specularExp = 0.0f;

        // Wireframe no-cull raster state
        D3D11_RASTERIZER_DESC rastDesc = {};
        rastDesc.FillMode = D3D11_FILL_WIREFRAME;
        rastDesc.CullMode = D3D11_CULL_NONE;
        device->CreateRasterizerState(&rastDesc, &wireframeMaterial.RasterStateOverride);

        MaterialIndex wireframeMaterialIndex = codex.PushMaterial(wireframeMaterial);
        assert(MI_WIREFRAME == wireframeMaterialIndex); // This is stupid  
    }

    return true;
}

}