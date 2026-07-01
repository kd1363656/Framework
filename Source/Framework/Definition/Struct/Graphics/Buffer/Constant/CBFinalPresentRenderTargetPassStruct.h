#pragma once

namespace FWK::Struct
{
	struct CBFinalPresentRenderTargetPass final
	{
		TypeAlias::DescriptorIndex m_sceneColorTextureSRVIndex = Constant::k_invalidDescriptorIndex;
		TypeAlias::Math::Vector3   m_padding				   = TypeAlias::Math::Vector3::Zero;
	};
}