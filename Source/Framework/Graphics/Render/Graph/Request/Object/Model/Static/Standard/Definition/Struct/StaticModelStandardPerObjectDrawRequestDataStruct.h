#pragma once

namespace FWK::Struct
{
	// 静的モデル標準描画用
	struct StaticModelStandardPerObjectDrawRequestData final
	{
		std::weak_ptr<Graphics::StaticModelRecord> m_staticModelRecord = {};

		TypeAlias::Math::Matrix m_worldMatrix = TypeAlias::Math::Matrix::Identity;
	};
}