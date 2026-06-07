#pragma once

namespace FWK::Struct
{
	struct TextureUploadRecord final
	{
		// 各サブリソースの配置情報をまとめたリスト
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> m_layoutList = {};

		// DEFAULTヒープ上のテクスチャリソースへコピーするための中間バッファ
		Graphics::UploadBuffer m_uploadBuffer = {};
	};

	struct TextureBatchUploadRecord final
	{
		std::shared_ptr<Graphics::TextureRecord> m_textureRecord       = nullptr;
		Struct::TextureUploadRecord			     m_textureUploadRecord = {};
	};
}