#pragma once

namespace FWK::Graphics
{
	class TextureSystem
	{
	public:

		 TextureSystem() = default;
		~TextureSystem() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create	    ();

		bool LoadTextureFile(const std::filesystem::path& a_filePath, const Enum::TextureLoadType a_loadType = Enum::TextureLoadType::Auto);

		nlohmann::json Serialize() const;

		const auto& GetREFTextureStorage() const { return m_textureStorage; }

		auto& GetMutableREFTextureStorage() { return m_textureStorage; }

	private:

		AssetStorage<Graphics::TextureRecord> m_textureStorage = {};

		TextureLoader m_loader = {};

		Converter::TextureSystemJsonConverter m_jsonConverter = {};
	};
}