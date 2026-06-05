#include "RenderGraphPassBase.h"

FWK::Graphics::RenderGraphPassBase::RenderGraphPassBase(const Enum::RenderGraphPassType a_renderGraphPassType) :
	m_renderGraphPassType(a_renderGraphPassType)
{}
FWK::Graphics::RenderGraphPassBase::~RenderGraphPassBase() = default;

void FWK::Graphics::RenderGraphPassBase::AddResourceAccess(const Enum::RenderGraphResourceType a_resourceType, const Enum::RenderGraphResourceAccessType a_accessType)
{
	Struct::RenderGraphResourceAccess l_resourceAccess = {};

	l_resourceAccess.m_resourceType = a_resourceType;
	l_resourceAccess.m_accessType   = a_accessType;

	m_resourceAccessList.emplace_back(l_resourceAccess);
}