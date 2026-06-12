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
								"RenderGraphResourceTypeが無効です、リソースアクセスの追加に失敗しました");

	FWK_ASSERT_RETURN_IF_FAILED(a_accessType == Enum::RenderGraphAccessType::Invalid ||
								a_accessType == Enum::RenderGraphAccessType::Count,
								"RenderGraphAccessTypeが無効です、リソースアクセスの追加に失敗しました");

	FWK_ASSERT_RETURN_IF_FAILED(a_usage == Enum::RenderGraphResourceUsage::Invalid ||
								a_usage == Enum::RenderGraphResourceUsage::Count,
								"RenderGraphResourceUsageが無効です、リソースアクセスの追加に失敗しました");

	Struct::RenderGraphResourceAccess l_resourceAccess = {};

	l_resourceAccess.m_resourceType = a_resourceType;
	l_resourceAccess.m_accessType	 = a_accessType;
	l_resourceAccess.m_usage		 = a_usage;

	m_resourceAccessList.emplace_back(l_resourceAccess);
}