#include "TextureSystem.h"

void FWK::Graphics::TextureSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::TextureSystem::Create()
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_textureStorage.Create(), "AssetStorageの作成に失敗したため、TextureSystemの作成処理に失敗しました。", false);

	return true;
}

bool FWK::Graphics::TextureSystem::LoadTextureFile(const std::filesystem::path& a_filePath, const Enum::TextureLoadType a_loadType)
{
	DirectX::TexMetadata  l_texMetadata  = {};
	DirectX::ScratchImage l_scratchImage = {};

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_loader.LoadTextureFile(a_filePath, 
															    a_loadType, 
																l_scratchImage,
																l_texMetadata),
																"テクスチャロード処理に失敗しました。",
																false);

	return true;
}

nlohmann::json FWK::Graphics::TextureSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}