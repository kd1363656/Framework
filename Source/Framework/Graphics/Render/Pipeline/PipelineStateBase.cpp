#include "PipelineStateBase.h"

void FWK::Graphics::PipelineStateBase::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }
}

bool FWK::Graphics::PipelineStateBase::Create(const Device & a_device, const ShaderCompiler & a_shaderCompiler, const Renderer & a_renderer)
{
	return false;
}

nlohmann::json FWK::Graphics::PipelineStateBase::Serialize() const
{
	return nlohmann::json();
}