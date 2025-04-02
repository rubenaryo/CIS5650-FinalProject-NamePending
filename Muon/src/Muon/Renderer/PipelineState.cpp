/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/4
Description : Implementation for Pipeline state objects (PSO)
----------------------------------------------*/

#include <Muon/Renderer/PipelineState.h>
#include <Muon/Renderer/ThrowMacros.h>
#include <Muon/Core/DXCore.h>

namespace Muon
{

bool GraphicsPipelineState::Generate()
{
	if (!GetDevice())
		return false;

	HRESULT hr = GetDevice()->CreateGraphicsPipelineState(&mDesc, IID_PPV_ARGS(&mpPipelineState));
	COM_EXCEPT(hr);

	return SUCCEEDED(hr);
}

}