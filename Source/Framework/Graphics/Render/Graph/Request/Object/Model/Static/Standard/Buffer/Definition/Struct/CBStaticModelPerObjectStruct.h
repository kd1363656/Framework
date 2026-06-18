#pragma once

namespace FWK::Struct
{
	struct CBStaticModelPerObject final
	{
		TypeAlias::Math::Matrix m_worldMatrix = TypeAlias::Math::Matrix::Identity;

		TypeAlias::StorageID m_baseColorTextureSRVDescriptorIndex = Constant::k_invalidDescriptorIndex;
		TypeAlias::StorageID m_normalTextureSRVDescriptorIndex    = Constant::k_invalidDescriptorIndex;
		TypeAlias::StorageID m_vertexBufferSRVDescriptorIndex     = Constant::k_invalidDescriptorIndex;
		TypeAlias::StorageID m_meshletBufferSRVDescriptorIndex    = Constant::k_invalidDescriptorIndex;

		TypeAlias::StorageID	 m_uniqueVertexIndexBufferSRVDescriptorIndex = Constant::k_invalidDescriptorIndex;
		TypeAlias::StorageID	 m_primitiveIndexBufferSRVDescriptorIndex    = Constant::k_invalidDescriptorIndex;
		TypeAlias::Math::Vector2 m_padding								     = {};
	};
}