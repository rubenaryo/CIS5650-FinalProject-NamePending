/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/2
Description : Declation of structs used as constant buffers by various shaders
----------------------------------------------*/
#ifndef CBUFFERSTRUCTS_H
#define CBUFFERSTRUCTS_H

#include <DirectXMath.h>
#include <DirectXColors.h>

namespace Muon
{

enum VSBindSlots
{
    VS_CAMERA_SLOT = 10,
    VS_ENTITY_SLOT = 11
};

enum PSBindSlots
{
    PS_LIGHTS_SLOT = 10,
    PS_MATERIAL_SLOT = 11
};

struct alignas(16) cbCamera
{
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 proj;
    DirectX::XMFLOAT4X4 viewProj;
};

struct alignas(16) cbPerEntity
{
    DirectX::XMFLOAT4X4 world;
};

struct alignas(16) cbMaterialParams
{
    DirectX::XMFLOAT4  colorTint = DirectX::XMFLOAT4(DirectX::Colors::Black);
    float              specularExp = 0.0f;
};

}
#endif