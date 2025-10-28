/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/4
Description : Implementation of custom material system
----------------------------------------------*/
#include "Material.h"

#include <Core/PipelineState.h>
#include <Core/Shader.h>

namespace Muon
{

ParameterDesc::ParameterDesc(const char* name, ParameterType type)
	:	Name(name)
	,	Type(type)
{
}

//////////

MaterialType::MaterialType(const char* name)
	:	mName(name)
{
}

void MaterialType::SetVertexShader(const VertexShader& vs)
{
	mpVS = &vs;
	mPipelineState.SetVertexShader(vs);
}

void MaterialType::SetPixelShader(const PixelShader& ps)
{
	mpPS = &ps;
	mPipelineState.SetPixelShader(ps);

	// TODO: Parse cbuffers automatically
}

void MaterialType::SetRootSignature(ID3D12RootSignature* pRootSig)
{
	mPipelineState.SetRootSignature(pRootSig);
}

// Added in-order, and it matters
void MaterialType::AddParameter(const char* paramName, ParameterType type)
{
	size_t index = mParameters.size();
	ParameterDesc& desc = mParameters.emplace_back(paramName, type);
	desc.Index = index;
	mParamNameToIndex[paramName] = index; // maybe this will be too cumbersome later..
}

const ParameterDesc* MaterialType::GetParameter(const char* paramName) const
{
	auto itFind = mParamNameToIndex.find(paramName);
	if (itFind == mParamNameToIndex.end() || itFind->second >= mParamNameToIndex.size())
		return nullptr;

	size_t index = itFind->second;
	return &mParameters.at(index);
}

bool MaterialType::Generate()
{
	return mPipelineState.Generate();
}

////////

MaterialInstance::MaterialInstance(const char* name, const MaterialType& materialType)
	:	mName(name)
	,	mType(materialType)
{
}

bool MaterialInstance::SetParamValue(const char* paramName, ParameterValue value)
{
	const ParameterDesc* pParamDesc = mType.GetParameter(paramName);
	if (!pParamDesc)
		return false;

	UINT index = pParamDesc->Index;
	mParamValues.reserve(index + 1);
	mParamValues[index] = value;
	return true;
}

size_t GetParamTypeSize(ParameterType type)
{
	static size_t sParamSizes[] =
	{
		sizeof(int),
		sizeof(float),
		sizeof(DirectX::XMFLOAT2),
		sizeof(DirectX::XMFLOAT3),
		sizeof(DirectX::XMFLOAT4),
	};
	static_assert(std::size(sParamSizes) == (UINT)ParameterType::Count);

	return sParamSizes[(UINT)type];
}

}
