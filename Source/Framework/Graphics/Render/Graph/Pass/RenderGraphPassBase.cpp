#include "RenderGraphPassBase.h"

void FWK::Graphics::RenderGraphPassBase::ReadResource(const Enum::RenderGraphResourceType a_resourceType, const Enum::RenderGraphResourceUsage a_usage)
{
	AddResourceAccess(a_resourceType, Enum::RenderGraphAccessType::Read, a_usage);
}

void FWK::Graphics::RenderGraphPassBase::WriteResource(const Enum::RenderGraphResourceType a_resourceType, const Enum::RenderGraphResourceUsage a_usage)
{
	AddResourceAccess(a_resourceType, Enum::RenderGraphAccessType::Write, a_usage);
}

void FWK::Graphics::RenderGraphPassBase::AddResourceAccess(const Enum::RenderGraphResourceType a_resourceType, const Enum::RenderGraphAccessType a_accessType, const Enum::RenderGraphResourceUsage a_usage)
{
	FWK_ASSERT_RETURN_IF_FAILED(a_resourceType == Enum::RenderGraphResourceType::Invalid ||
								a_resourceType == Enum::RenderGraphResourceType::Count, 
								"RenderGraphResourceTypeが無効です。");

	FWK_ASSERT_RETURN_IF_FAILED(a_accessType == Enum::RenderGraphAccessType::Invalid ||
								a_accessType == Enum::RenderGraphAccessType::Count,
								"RenderGraphResourceUsageが無効です。");

	Struct::RenderGraphResourceAccess l_resourceAcccess = {};

	l_resourceAcccess.m_resourceType = a_resourceType;
	l_resourceAcccess.m_accessType	 = a_accessType;
	l_resourceAcccess.m_usage		 = a_usage;

	m_resourceAccecssList.emplace_back(l_resourceAcccess);
}