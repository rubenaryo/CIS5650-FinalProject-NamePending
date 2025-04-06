/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/3
Description : Implementation of Renderer class
----------------------------------------------*/
#include "EntityRenderer.h"

#include "Camera.h"
#include "CBufferStructs.h"
#include "ConstantBuffer.h"
#include "DeviceResources.h"
#include "DrawContext.h"
#include "hash_util.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourceCodex.h"
#include "Shader.h"
#include "SkyRenderer.h"
#include "ThrowMacros.h"

#if defined(MN_DEBUG)
#include <typeinfo>
#endif

#include <random>
#include <time.h>

namespace Renderer {

EntityRenderer::EntityRenderer()
{}

void EntityRenderer::Init(DeviceResources const& dr)
{
    // Grab reference to d3d11 device and context
    auto device = dr.GetDevice();
    auto context = dr.GetContext();

    // Initialize meshes, materials, entities
    InitMeshes(dr);
    InitEntities();
    InitDrawContexts(device);
    
    // For now, assume we're only using trianglelist
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ConstantBufferUpdateManager::Populate(sizeof(cbMaterialParams), (UINT)PS_REGISTERS::MATERIAL, EASEL_SHADER_STAGE::ESS_PS, device, &MaterialParamsCB);
    ConstantBufferUpdateManager::Bind(&MaterialParamsCB, context);

    ConstantBufferUpdateManager::Populate(sizeof(cbPerEntity), (UINT)VS_REGISTERS::WORLD, EASEL_SHADER_STAGE::ESS_VS, device, &EntityCB);
    ConstantBufferUpdateManager::Bind(&EntityCB, context);
}

//TODO: Lots of hardcoded hashes here huh
void EntityRenderer::InitMeshes(DeviceResources const& dr)
{
    auto device = dr.GetDevice();

    ResourceCodex const& sg_Codex = ResourceCodex::GetSingleton();

    const ShaderID kInstancedPhongVSID = 0xc8a366aa; // FNV1A of L"InstancedPhongVS.cso"
    const ShaderID kPhongPSID = 0x4dc6e249;          // FNV1A of L"PhongPS.cso"
    const ShaderID kPhongPSNormalMapID = fnv1a(L"Phong_NormalMapPS.cso");

    const VertexShader* instancedPhongVS = sg_Codex.GetVertexShader(kInstancedPhongVSID);
    const PixelShader*  PhongPS = sg_Codex.GetPixelShader(kPhongPSID);

    const VertexBufferDescription* phongVertDesc = &instancedPhongVS->VertexDesc;
    const MeshID sphereID = ResourceCodex::AddMeshFromFile("sphere.obj", phongVertDesc);
    const MeshID cubeID = ResourceCodex::AddMeshFromFile("cube.obj", phongVertDesc);
    
    //dr.GetContext()->PSSetSamplers(0, 1, &PhongPS->SamplerState);
}

void EntityRenderer::InitEntities()
{
    UINT width = 20;
    UINT height = width;
    
    const UINT kNumEntities = width * height;
    EntityCount = kNumEntities;

    Entities = (Entity*)malloc(sizeof(Entity) * kNumEntities);
    MeshID cubeMeshId = fnv1a(L"cube.obj");

    UINT entityIdx = 0;
    for (UINT i = 0; i != width; ++i)
    {
        for (UINT j = 0; j != height; ++j)
        {
            Core::Transform tfm;
            tfm.SetTranslation((float)i, 0.0f, (float)j);

            Entity test;
            test.MaterialIndex = i == 0 && j == 0 ? MI_WIREFRAME : MI_LUNAR; 
            test.mMeshID = cubeMeshId;
            test.mTransform = tfm;

            Entities[entityIdx++] = test;
        }
    }
}

void EntityRenderer::InitDrawContexts(ID3D11Device* device)
{
    const UINT kInstancingPassCount = 1;

    InstancingPassCount = kInstancingPassCount;
    InstancingPasses = new InstancedDrawContext[kInstancingPassCount];

    InstancedDrawContext& cubeDraw = InstancingPasses[0];
    cubeDraw.InstanceCount   = EntityCount;
    cubeDraw.WorldMatrices   = (DirectX::XMFLOAT4X4*)malloc(sizeof(DirectX::XMFLOAT4X4) * cubeDraw.InstanceCount);
    cubeDraw.InstancedMeshID = Entities[0].mMeshID;
    cubeDraw.MaterialIndex   = MI_LUNAR;

    static const bool DRAW_WIREFRAME = true;
    if (DRAW_WIREFRAME)
    {
        cubeDraw.MaterialIndex = MI_WIREFRAME;
    }

    // Create the dynamic vertex buffer
    D3D11_BUFFER_DESC dynamicDesc = {0};
    dynamicDesc.Usage = D3D11_USAGE_DYNAMIC;
    dynamicDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    dynamicDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    dynamicDesc.MiscFlags = 0;
    dynamicDesc.StructureByteStride = 0;
    dynamicDesc.ByteWidth = sizeof(DirectX::XMFLOAT4X4) * cubeDraw.InstanceCount;
    COM_EXCEPT(device->CreateBuffer(&dynamicDesc, nullptr, &cubeDraw.DynamicBuffer));
}

void EntityRenderer::Update(ID3D11DeviceContext* context, float dt)
{
    using namespace DirectX;
    using Core::Transform;

    const float rotSpeed = 1.25f;

    static const XMVECTOR rot1 = DirectX::XMQuaternionRotationRollPitchYaw(rotSpeed, -rotSpeed, rotSpeed);
    static const XMVECTOR rot2 = -rot1;

    InstancedDrawContext& lunarDraw = InstancingPasses[0];

    for (UINT i = 0; i != EntityCount; ++i)
    {
        Transform* tfm = &Entities[i].mTransform;
        lunarDraw.WorldMatrices[i] = tfm->Recompute();
    }

    // Rewrite the dynamic vertex buffer
    D3D11_MAPPED_SUBRESOURCE mappedBuffer;
    COM_EXCEPT(context->Map(lunarDraw.DynamicBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer));
    memcpy(mappedBuffer.pData, lunarDraw.WorldMatrices, sizeof(DirectX::XMFLOAT4X4) * InstancingPasses[0].InstanceCount);
    context->Unmap(lunarDraw.DynamicBuffer, 0);
}

void EntityRenderer::Draw(ID3D11DeviceContext* context)
{
    this->InstancedDraw(context);
}

void EntityRenderer::InstancedDraw(ID3D11DeviceContext* context)
{
    ResourceCodex const& sg_Codex = ResourceCodex::GetSingleton();

    InstancedDrawContext* drawCtx = InstancingPasses; 
    InstancedDrawContext* const drawCtxItEnd = InstancingPasses + InstancingPassCount;
    for (; drawCtx != drawCtxItEnd; ++drawCtx)
    {
        //const Mesh* const mesh = sg_Codex.GetMesh(drawCtx->InstancedMeshID);
        const Mesh* mesh = sg_Codex.GetMesh(drawCtx->InstancedMeshID);

        ID3D12Resource* vertBuffers[2];
        vertBuffers[0] = mesh->VertexBuffer;        // Vertices
        //vertBuffers[1] = drawCtx->DynamicBuffer;    // Instanced World Matrices

        static const UINT strides[2] = 
        {
            mesh->Stride,
            sizeof(DirectX::XMFLOAT4X4)
        };

        static const UINT offsets[2] = 
        {
            0, 
            0
        };


        //context->IASetVertexBuffers(0, 2, &vertBuffers[0], &strides[0], &offsets[0]);
        //context->IASetIndexBuffer(mesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Setup VS,PS
        const Material mat = *sg_Codex.GetMaterial(drawCtx->MaterialIndex);
        const VertexShader* VS = mat.VS;
        const PixelShader*  PS = mat.PS;

        ID3D11RasterizerState* pCurrRasterState = nullptr;
        ID3D11RasterizerState* pRasterStateOverride = mat.RasterStateOverride;
        if (pRasterStateOverride)
        {
            context->RSGetState(&pCurrRasterState);
            context->RSSetState(pRasterStateOverride);
        }

        //context->IASetInputLayout(VS->InputLayout);
        //context->VSSetShader(VS->Shader, nullptr, 0);
        //context->PSSetShader(PS->Shader, nullptr, 0);

        // Update Material Param Data:
        ConstantBufferUpdateManager::MapUnmap(&MaterialParamsCB, (void*)&mat.Description, context);

        // Bind Textures expected by the shader
        if (mat.Resources)
            context->PSSetShaderResources(0, (UINT)TextureSlots::COUNT, mat.Resources->SRVs);

        // Submit draw call to GPU
        context->DrawIndexedInstanced(mesh->IndexCount, drawCtx->InstanceCount, 0, 0, 0);

        if (pRasterStateOverride)
        {
            context->RSSetState(pCurrRasterState); // Put things back how they were
        }
    }
}

EntityRenderer::~EntityRenderer()
{
    for (uint8_t i = 0; i != 1; ++i)
    {
        InstancedDrawContext& drawCtx = InstancingPasses[i];

        free(drawCtx.WorldMatrices);
        drawCtx.WorldMatrices = nullptr;
        drawCtx.DynamicBuffer->Release();
    }
    
    delete[] InstancingPasses;
    InstancingPasses = nullptr;

    free(Entities);

    ConstantBufferUpdateManager::Cleanup(&MaterialParamsCB);
    ConstantBufferUpdateManager::Cleanup(&EntityCB);
    
    ResourceCodex::Destroy();
}
}