/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/4
Description : Implementation for Pipeline state objects (PSO)
----------------------------------------------*/

#include <Muon/Renderer/PipelineState.h>
#include <Muon/Renderer/Shader.h>
#include <Muon/Renderer/ThrowMacros.h>
#include <Muon/Core/DXCore.h>

namespace Muon
{

bool PipelineState::Bind() const
{
    ID3D12GraphicsCommandList* pCommandList = GetCommandList();
    if (!pCommandList || !this->GetPipelineState())
        return false;

    pCommandList->SetPipelineState(this->GetPipelineState());
    return true;
}

GraphicsPipelineState::GraphicsPipelineState()
{
    // Set some default values
    mDesc = { 0 };
    mDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    mDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    mDesc.DepthStencilState.DepthEnable = FALSE;
    mDesc.DepthStencilState.StencilEnable = FALSE;
    mDesc.SampleMask = UINT_MAX;
    mDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    mDesc.NumRenderTargets = 1;
    mDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    mDesc.SampleDesc.Count = 1;
}

void GraphicsPipelineState::SetRootSignature(ID3D12RootSignature* pRootSig)
{
    PipelineState::SetRootSignature(pRootSig);
    mDesc.pRootSignature = pRootSig;
}

void GraphicsPipelineState::SetVertexShader(const Renderer::VertexShader& vs)
{
    mDesc.InputLayout = { vs.InputElements.data(), (unsigned int) vs.InputElements.size()};
    mDesc.VS = CD3DX12_SHADER_BYTECODE(vs.ShaderBlob.Get());
}

void GraphicsPipelineState::SetPixelShader(const Renderer::PixelShader& ps)
{
    mDesc.PS = CD3DX12_SHADER_BYTECODE(ps.ShaderBlob.Get());
}

bool GraphicsPipelineState::Generate()
{
	if (!GetDevice())
		return false;

	HRESULT hr = GetDevice()->CreateGraphicsPipelineState(&mDesc, IID_PPV_ARGS(&mpPipelineState));
	COM_EXCEPT(hr);

	return SUCCEEDED(hr);
}


}