#include "TextureLoader.h"

bool FWK::Graphics::TextureLoader::LoadTextureFile(const std::filesystem::path& a_filePath, 
												   const Enum::TextureLoadType  a_textureLoadType, 
														 DirectX::ScratchImage& a_scratchImage, 
														 DirectX::TexMetadata&  a_texMetadata)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension), "ロードしようとしたファイルが無効かPNGファイルでないため、テクスチャファイル読み込みに失敗しました。", false);

	const auto l_wicFlags = CreateWICFlags(a_textureLoadType);

	// PNGなどの標準的な画像から情報を取得する関数
	// LoadFromWICFile(読み込み画像ファイルパス、
	//                 WIC読み込み時の追加フラグ、
	//                 画像の幅、高さ、mip数、DXGI_FORMATなどを受け取る構造体、
	//                 実際の画像ピクセルデータを受け取るScratchImage);
	const auto l_hr = DirectX::LoadFromWICFile(a_filePath.c_str(),
											   l_wicFlags,
											   &a_texMetadata,
											   a_scratchImage);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr),													   "PNGファイルからテクスチャを読み込めませんでした。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_binaryConverter.SaveTextureAsset(a_filePath, a_scratchImage), "TextureAssetの保存に失敗しました",					false);

	return true;
}

DirectX::WIC_FLAGS FWK::Graphics::TextureLoader::CreateWICFlags(const Enum::TextureLoadType a_textureLoadType) const
{
	switch (a_textureLoadType)
	{
		case Enum::TextureLoadType::Color:
		{
			// BasColor/Albedo/Diffuseのような「色」として扱うテクスチャ
			return DirectX::WIC_FLAGS_FORCE_SRGB;
		}
		break;

		case Enum::TextureLoadType::Normal:
		case Enum::TextureLoadType::Data:
		{
			// Normal/Metallic/Roughness/AOなどは色ではなく数値データ。
			// sRGB補正が入ると値が壊れるため、Linear扱いにする
			return DirectX::WIC_FLAGS_IGNORE_SRGB;
		}
		break;

		case Enum::TextureLoadType::Auto:
		default:
		{
			// WIC/DirectXTex側の標準判定に任せる
			return DirectX::WIC_FLAGS_NONE;
		}
		break;
	}
}