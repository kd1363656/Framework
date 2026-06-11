#pragma once

namespace FWK::Struct
{
	struct RenderGraphResourceAccess final 
	{
		Enum::RenderGraphResourceType  m_resourceType = Enum::RenderGraphResourceType::Invalid;
		Enum::RenderGraphAccessType    m_accessType   = Enum::RenderGraphAccessType::Invalid;
		Enum::RenderGraphResourceUsage m_usage        = Enum::RenderGraphResourceUsage::Invalid;
	};
}