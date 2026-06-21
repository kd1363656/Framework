#include "PassTextureBase.h"

UINT FWK::Graphics::PassTextureBase::FetchVALPassTextureWidth(const UINT a_clientWidth) const
{
	if (m_isFixedSize) { return m_width; }

	return a_clientWidth;
}
UINT FWK::Graphics::PassTextureBase::FetchVALPassTextureHeight(const UINT a_clientHeight) const
{
	if (m_isFixedSize) { return m_height; }

	return a_clientHeight;
}