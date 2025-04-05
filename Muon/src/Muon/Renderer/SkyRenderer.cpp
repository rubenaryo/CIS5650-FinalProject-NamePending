/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/9
Description : Handles the rendering of the sky backdrop
----------------------------------------------*/
#include "SkyRenderer.h"

#include "ResourceCodex.h"
#include "ThrowMacros.h"

#include "Mesh.h"
#include "Shader.h"
#include "Material.h"

namespace Renderer {

void SkyRenderer::Init(ID3D11Device* device)
{
    // Mesh, texture, and shaders
    ResourceCodex const& codex = ResourceCodex::GetSingleton();
    const MeshID kSkyMeshID = 0x4a986f37;

    // Query the resource codex to get the bindables directly.
    const Material* pSkyMaterial = codex.GetMaterial(MI_SKY);
    if (!pSkyMaterial)
    {
        // failed to get skybox material
        assert(false);
        return;
    }

    SkyMaterialCopy = *pSkyMaterial;
    //CubeMesh = codex.GetMesh(kSkyMeshID);

    assert(SkyMaterialCopy.VS);
    assert(SkyMaterialCopy.PS);
    assert(SkyMaterialCopy.RasterStateOverride);
    assert(SkyMaterialCopy.DepthStencilStateOverride);
    assert(SkyMaterialCopy.Resources);
    assert(SkyMaterialCopy.Resources->SRVs);

    SkyMaterialCopy.RasterStateOverride->AddRef();
    SkyMaterialCopy.DepthStencilStateOverride->AddRef();
}

void SkyRenderer::Draw(ID3D11DeviceContext* context)
{
    // Set backface culling
    ID3D11DepthStencilState* pCurrDepthStencilState = nullptr;
    context->OMGetDepthStencilState(&pCurrDepthStencilState, nullptr);
    context->OMSetDepthStencilState(SkyMaterialCopy.DepthStencilStateOverride, 0);

    ID3D11RasterizerState* pCurrRasterState = nullptr;
    context->RSGetState(&pCurrRasterState);
    context->RSSetState(SkyMaterialCopy.RasterStateOverride);

    UINT offsets = 0;

    // Bind the Cube Mesh
    const Mesh mesh = *CubeMesh;
    context->IASetVertexBuffers(0, 1, &mesh.VertexBuffer, &mesh.Stride, &offsets);
    context->IASetIndexBuffer(mesh.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set Vertex Shader and Input
    //context->IASetInputLayout(SkyMaterialCopy.VS->InputLayout);
    //context->VSSetShader(SkyMaterialCopy.VS->Shader, 0, 0);
    //
    //// Set Pixel Shader and Bind Textures
    //context->PSSetShaderResources(0, (UINT)TextureSlots::COUNT, SkyMaterialCopy.Resources->SRVs);
    //context->PSSetShader(SkyMaterialCopy.PS->Shader, 0, 0);

    // Submit Draw Call
    context->DrawIndexed(mesh.IndexCount, 0, 0);

    // Reset states back to previous
    context->OMSetDepthStencilState(pCurrDepthStencilState, 0);
    context->RSSetState(0);
}

SkyRenderer::~SkyRenderer()
{
    if (SkyMaterialCopy.RasterStateOverride)
        SkyMaterialCopy.RasterStateOverride->Release();

    if (SkyMaterialCopy.DepthStencilStateOverride)
        SkyMaterialCopy.DepthStencilStateOverride->Release();
}

}