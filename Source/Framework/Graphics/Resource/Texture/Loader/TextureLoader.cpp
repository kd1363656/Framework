#include "TextureLoader.h"

bool FWK::Graphics::TextureLoader::LoadTextureFile(const std::filesystem::path& a_filePath, DirectX::ScratchImage& a_scratchImage, DirectX::TexMetadata& a_texMetadata) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension), "ロードしようとしたファイルが無効かPNGファイルでないため、テクスチャファイル読み込みに失敗しました。", false);

	// PNGなどの標準的な画像から情報を取得する関数
	// LoadFromWICFile(読み込み画像ファイルパス、
	//                 WIC読み込み時の追加フラグ、
	//                 画像の幅、高さ、mip数、DXGI_FORMATなどを受け取る構造体、
	//                 実際の画像ピクセルデータを受け取るScratchImage);
	
	return true;
}