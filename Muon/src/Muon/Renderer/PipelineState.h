/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/4
Description : Interface for Pipeline state objects (PSO)
----------------------------------------------*/
#ifndef MUON_PIPELINESTATE_H
#define MUON_PIPELINE_STATE_H

#include <d3d12.h>

namespace Muon
{

class PipelineState
{
public:	
	void SetRootSignature(const ID3D12RootSignature* pRootSig) { mpRootSignature = pRootSig; }
	const ID3D12RootSignature* GetRootSignature() const { return mpRootSignature; }
	ID3D12PipelineState* GetPipelineState() const { return mpPipelineState; }

protected:
	const ID3D12RootSignature* mpRootSignature = nullptr;
	ID3D12PipelineState* mpPipelineState = nullptr;
};

class GraphicsPipelineState : public PipelineState
{
public:
	// Generates the mpPipelineState from the parameters held in the description
	bool Generate();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC mDesc;
};

}

#endif