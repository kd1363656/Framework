#pragma once

namespace FWK::Struct
{
	struct TextureUploadRecord final
	{
		 TextureUploadRecord() = default;
		~TextureUploadRecord() = default;

		TextureUploadRecord(const TextureUploadRecord&)           = delete;
		TextureUploadRecord(	  TextureUploadRecord&&) noexcept = default;

		TextureUploadRecord& operator=(const TextureUploadRecord&)			 = delete;
		TextureUploadRecord& operator=(	     TextureUploadRecord&&) noexcept = default;

		// 各サブリソースの配置情報をまとめたリスト
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> m_layoutList = {};

		// DEFAULTヒープ上のテクスチャリソースへコピーするための中間バッファ
		Graphics::UploadBuffer m_uploadBuffer = {};
	};

	struct TextureBatchUploadRecord final
	{
		 TextureBatchUploadRecord() = default;
		~TextureBatchUploadRecord() = default;

		TextureBatchUploadRecord(const TextureBatchUploadRecord&)           = delete;
		TextureBatchUploadRecord(	   TextureBatchUploadRecord&&) noexcept = default;

		TextureBatchUploadRecord& operator=(const TextureBatchUploadRecord&)		   = delete;
		TextureBatchUploadRecord& operator=(	  TextureBatchUploadRecord&&) noexcept = default;

		std::shared_ptr<Graphics::TextureRecord> m_textureRecord       = nullptr;
		Struct::TextureUploadRecord			     m_textureUploadRecord = {};
	};
}