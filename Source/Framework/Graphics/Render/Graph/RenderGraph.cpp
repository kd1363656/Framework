#include "RenderGraph.h"

void FWK::Graphics::RenderGraph::INIT()
{
}

void FWK::Graphics::RenderGraph::BuildDefaultBackBufferGraph()
{
}

bool FWK::Graphics::RenderGraph::Compile()
{
	return false;
}

void FWK::Graphics::RenderGraph::BeginFrame()
{
}

void FWK::Graphics::RenderGraph::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer)
{
}

void FWK::Graphics::RenderGraph::AddPass(std::unique_ptr<RenderGraphPassBase>&& a_pass)
{
}

void FWK::Graphics::RenderGraph::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Graphics::RenderGraph::Serialize() const
{
	return nlohmann::json();
}

bool FWK::Graphics::RenderGraph::IsReadAccess(const Enum::RenderGraphResourceAccessType a_accessType) const
{
	return false;
}
bool FWK::Graphics::RenderGraph::IsWriteAccess(const Enum::RenderGraphResourceAccessType a_accessType) const
{
	return false;
}
bool FWK::Graphics::RenderGraph::IsFinalStateAccess(const Enum::RenderGraphResourceAccessType a_accessType) const
{
	return false;
}

void FWK::Graphics::RenderGraph::AddDependencyEdge(const std::uint32_t a_fromPassIndex, const std::uint32_t a_toPassIndex, std::vector<std::vector<std::uint32_t>>& a_edgeList, std::vector<std::uint32_t>& a_inDegreelist) const
{
}

void FWK::Graphics::RenderGraph::BuildDependency(std::vector<std::vector<std::uint32_t>>& a_edgeList, std::vector<std::uint32_t>& a_inDegreeList) const
{
}

void FWK::Graphics::RenderGraph::TransitionPassResources(const RenderGraphPassBase& a_pass, Renderer& a_renderer)
{
}
void FWK::Graphics::RenderGraph::TransitionBackBufferResource(const RenderGraphPassBase& a_pass, Renderer& a_renderer)
{
}

D3D12_RESOURCE_STATES FWK::Graphics::RenderGraph::ConvertAccessTypeToResourceState(const Enum::RenderGraphResourceAccessType a_accessType) const
{
	return D3D12_RESOURCE_STATES();
}

void FWK::Graphics::RenderGraph::SetupCurrentResourceState(const Enum::RenderGraphResourceType a_resourceType)
{
}