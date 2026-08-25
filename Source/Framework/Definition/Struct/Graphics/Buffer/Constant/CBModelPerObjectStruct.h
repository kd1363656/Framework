#pragma once

namespace FWK::Struct
{
	struct CBModelPerObject final
	{
		TypeAlias::Math::Matrix m_worldMatrix = TypeAlias::Math::Matrix::Identity;

		TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

		TypeAlias::Math::Color m_baseColorFactor = Constant::k_whiteColor;

		float				       m_roughnessFactor					= ModelMaterialAssetData::k_defaultModelMaterialRoughnessFactor;
		float					   m_metallicFactor					    = ModelMaterialAssetData::k_defaultModelMaterialMetallicFactor;
		TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_normalTextureSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

		TypeAlias::DescriptorIndex m_metallicTextureSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_roughnessTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex     = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

		TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex     = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		float					   m_worldMaxScale							   = Constant::k_defaultMeshletBoundWorldMaxScale;

		float                    m_worldOrientationSign = Constant::k_normalModelWorldOrientationSign;
		std::uint32_t            m_meshletCount         = Constant::k_defaultMeshletCount;
		TypeAlias::Math::Vector2 m_padding              = TypeAlias::Math::Vector2::Zero;
	};
}