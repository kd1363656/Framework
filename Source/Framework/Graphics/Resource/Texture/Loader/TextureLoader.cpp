#include "TextureLoader.h"

bool FWK::Graphics::TextureLoader::LoadTextureFile(const std::filesystem::path&      a_filePath, 
												   const Enum::TextureLoadColorSpace a_textureLoadColorSpace, 
														 DirectX::ScratchImage&      a_scratchImage, 
														 DirectX::TexMetadata&       a_texMetadata) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension), "ロードしようとしたファイルが無効かPNGファイルでないため、テクスチャファイル読み込みに失敗しました。", false);

	const auto l_wicFlags = CreateWICFlags(a_textureLoadColorSpace);

	// PNGなどの標準的な画像から情報を取得する関数
	// LoadFromWICFile(読み込み画像ファイルパス、
	//                 WIC読み込み時の追加フラグ、
	//                 画像の幅、高さ、MIP数、DXGI_FORMATなどを受け取る構造体、
	//                 実際の画像ピクセルデータを受け取るScratchImage);
	const auto l_hr = DirectX::LoadFromWICFile(a_filePath.c_str(),
											   l_wicFlags,
											   &a_texMetadata,
											   a_scratchImage);

	FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "PNGファイルからテクスチャを読み込めませんでした。", false);
	
	return true;
}

DirectX::WIC_FLAGS FWK::Graphics::TextureLoader::CreateWICFlags(const Enum::TextureLoadColorSpace a_textureLoadColorSpace) const
{
	switch (a_textureLoadColorSpace)
	{
		case Enum::TextureLoadColorSpace::SRGB:
		{
			return DirectX::WIC_FLAGS_FORCE_SRGB;
		}
		break;

		case Enum::TextureLoadColorSpace::Linear:
		{
			//  2D画像/Normal/Metallic/Roughness/AOなどは色ではなく数値データ。
			// sRGB補正が入ると値が壊れるため、Linear扱いにする
			return DirectX::WIC_FLAGS_IGNORE_SRGB;
		}
		break;

		case Enum::TextureLoadColorSpace::Auto:
		default:
		{
			// WIC/DirectXTex側の標準判定に任せる
			return DirectX::WIC_FLAGS_NONE;
		}
		break;
	}
}