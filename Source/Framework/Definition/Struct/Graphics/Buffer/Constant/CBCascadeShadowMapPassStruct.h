#pragma once

namespace FWK::Struct
{
	struct CBCascadeShadowMapPass final
	{
		std::array<TypeAlias::Math::Matrix, Constant::k_cascadeShadowMapDefaultMAXCascadeCount> m_viewProjectionMatrixList = {};
		std::array<float,                   Constant::k_cascadeShadowMapDefaultMAXCascadeCount> m_splitDepthList           = {};

		TypeAlias::DescriptorIndex m_shadowMapSRVDeccriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		UINT                       m_cascadeCount                = {};
		float                      m_sampleDepthBias             = Constant::k_cascadeShadowMapDefaultSampleDepthBias;
		float                      m_padding                     = 0.0F;
	};
}