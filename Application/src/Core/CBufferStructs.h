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

struct alignas(16) cbCamera
{
    DirectX::XMFLOAT4X4 viewProjection;
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