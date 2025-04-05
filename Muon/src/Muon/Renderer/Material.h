/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/2
Description : Material class for shader information
----------------------------------------------*/
#ifndef MATERIAL_H
#define MATERIAL_H

#include "DXCore.h"
#include "CBufferStructs.h"

namespace Renderer
{
struct VertexShader;
struct PixelShader;
struct GraphicsPipelineState;
struct VertexShader_DX12;
struct PixelShader_DX12;
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

struct ResourceBindChord
{
    ID3D11ShaderResourceView*  SRVs[(UINT)TextureSlots::COUNT];
};

// Materials own both VS and PS because they must match in the pipeline
struct Material
{
    const VertexShader*         VS = nullptr;
    const PixelShader*          PS = nullptr;;
    const ResourceBindChord*    Resources = nullptr;
    ID3D11RasterizerState*      RasterStateOverride = nullptr;
    ID3D11DepthStencilState*    DepthStencilStateOverride = nullptr;
    cbMaterialParams            Description;
};

struct Material_DX12
{
    const VertexShader_DX12* VS = nullptr;
    const PixelShader_DX12* PS = nullptr;
};
    

}
#endif