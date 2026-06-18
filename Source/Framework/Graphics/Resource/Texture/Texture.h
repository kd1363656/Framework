#pragma once

namespace FWK::Graphics
{
	class Texture final
	{
	public:
		
		 Texture();
		 Texture(const Texture&  a_other);
		 Texture(	   Texture&& a_other) noexcept;
		~Texture();

		Texture& operator=(const Texture&  a_other);
		Texture& operator=(		 Texture&& a_other) noexcept;

		bool Load(const std::filesystem::path& a_filePath, Enum::TextureLoadType a_loadType = Enum::TextureLoadType::Auto, Enum::DefaultTextureType a_defaultTextureType = Enum::DefaultTextureType::BaseColor);

		const auto& GetREFTextureRecord() const { return m_textureRecord; }

		auto GetVALStorageID() const { return m_storageID; }

	private:

		void AddTextureReferenceCount() const;

		void SubtractTextureReferenceCount();

		std::weak_ptr<Graphics::TextureRecord> m_textureRecord = {};

		TypeAlias::StorageID m_storageID;
	};
}