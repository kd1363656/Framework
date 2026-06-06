#include "TextureSystem.h"

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