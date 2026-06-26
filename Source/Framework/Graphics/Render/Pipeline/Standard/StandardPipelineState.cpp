#include "StandardPipelineState.h"


void FWK::Graphics::StandardPipelineState::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	PipelineStateBase::Deserialize(a_rootJson);
	m_jsonConverter.Deserialize   (a_rootJson, *this);
}

bool FWK::Graphics::StandardPipelineState::Create(const Device& a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer)
{
	return true;
}

nlohmann::json FWK::Graphics::StandardPipelineState::Serialize() const
{
	auto l_rootJson = PipelineStateBase::Serialize();

	Utility::UpdateJson(l_rootJson, m_jsonConverter.Serialize(*this));

	return l_rootJson;
}