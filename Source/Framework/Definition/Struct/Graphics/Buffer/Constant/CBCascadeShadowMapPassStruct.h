#pragma once

namespace FWK::Struct
{
	struct CBCascadeShadowMapPass final
	{
		std::array<TypeAlias::Math::Matrix, Constant::k_cascadeShadowMapDefaultMAXCascadeCount> m_viewProjectionMatrixList = {};
		std::array<float,                   Constant::k_cascadeShadowMapDefaultMAXCascadeCount> m_splitDepthList           = {};

		float m_sampleDepthBias                                  = Constant::k_cascadeShadowMapDefaultSampleDepthBias;
		float m_padding                                          = 0.0F;
		TypeAlias::DescriptorIndex m_shadowMapSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		UINT                       m_cascadeCount                = {};
	};
}