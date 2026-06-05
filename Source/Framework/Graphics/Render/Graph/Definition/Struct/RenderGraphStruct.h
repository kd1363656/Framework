#pragma once

namespace FWK::Struct
{
	struct RenderGraphResourceAccess final
	{	
		Enum::RenderGraphResourceType		m_resourceType = Enum::RenderGraphResourceType::None;
		Enum::RenderGraphResourceAccessType m_accessType   = Enum::RenderGraphResourceAccessType::None;
	};
}