#include "TextureBinaryConverter.h"

bool FWK::Converter::TextureBinaryConverter::SaveTextureAsset(const std::filesystem::path& a_filePath, const DirectX::ScratchImage& a_scratchImage)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension), "TextureAssetの元になるPNGファイルが無効です。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!IsValidScratchImage	   (a_scratchImage),							"TextureAssetへ保持するScratchImageが無効です",	 false);

	const auto& l_textureAssetFilePath = CreateTextureAssetFilePath   (a_filePath);
	const auto& l_textureAssetFileSize = CalculateTextureAssetFileSize(a_scratchImage);

	// 書き込むようメモリマップドファイルの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateWriteMemoryMappedFile(l_textureAssetFilePath, l_textureAssetFileSize), "TextureAssetの書き込み用MemoryMappedFile作成に失敗しました。", false);

	// 作成したメモリマップマップドデータを取得
	auto* l_mappedData = GetMutablePTRMappedData();

	if (!l_mappedData)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetの書き込み先メモリが無効です。", false);
	}

	auto l_memoryWriteOffset = GetREFInitialMemoryWriteOffset();

	// メモリマップドファイル書き込み用テクスチャバイナリヘッダーを作成
	const auto& l_textureBinaryHeader = CreateTextureBinaryHeader(a_scratchImage, l_textureAssetFileSize);

	WriteBinaryData(GetREFSingleBinaryElementCount(),
					&l_textureBinaryHeader,
					l_memoryWriteOffset,
					l_mappedData);

	// ScratchImageからイメージリストを取得
	const auto* l_imageList = a_scratchImage.GetImages();

	if (!l_imageList)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetへ保存するImage配列が無効です。", false);
	}

	const auto l_imageCount = a_scratchImage.GetImageCount();

	for (std::uint64_t l_imageIndex = 0ULL; l_imageIndex < l_imageCount; ++l_imageIndex)
	{
		// テクスチャイメージからサブリソースヘッダーを作成し書き込む
		const auto& l_image							 = l_imageList[l_imageIndex];
		const auto& l_textureBinarySubresourceHeader = CreateTextureBinarySubresuourceHeader(l_image);

		WriteBinaryData(GetREFSingleBinaryElementCount(),
						&l_textureBinarySubresourceHeader,
						l_memoryWriteOffset,
						l_mappedData);

		WriteBinaryData(l_textureBinarySubresourceHeader.m_pixelDataSize,
						l_image.pixels,
						l_memoryWriteOffset,
						l_mappedData);
	}

	if (l_memoryWriteOffset != l_textureAssetFileSize)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetの書き込みサイズが計算したファイルサイズと一致しません。", false);
	}

	DestroyMemoryMappedFile();

	return true;
}

std::filesystem::path FWK::Converter::TextureBinaryConverter::CreateTextureAssetFilePath(const std::filesystem::path& a_filePath) const
{
	return Utility::CreateFilePathByReplaceExtension(a_filePath, Constant::k_lowerAssetExtension);
}

FWK::Converter::TextureBinaryConverter::TextureBinaryHeader FWK::Converter::TextureBinaryConverter::CreateTextureBinaryHeader(const DirectX::ScratchImage& a_scratchImage, const std::uint64_t& a_filesize) const
{
	TextureBinaryHeader l_textureBinaryHeader = {};

	const auto& l_texMetadata = a_scratchImage.GetMetadata();

	l_textureBinaryHeader.m_fileSize    = a_filesize;
	l_textureBinaryHeader.m_version     = k_textureAssetVersion;
	l_textureBinaryHeader.m_assetTypeID = k_textureAssetTypeID;

	l_textureBinaryHeader.m_width			 = l_texMetadata.width;
	l_textureBinaryHeader.m_height			 = l_texMetadata.height;
	l_textureBinaryHeader.m_depth			 = l_texMetadata.depth;
	l_textureBinaryHeader.m_arraySize		 = l_texMetadata.arraySize;
	l_textureBinaryHeader.m_mipLevels		 = l_texMetadata.mipLevels;
	l_textureBinaryHeader.m_subresourceCount = a_scratchImage.GetImageCount();

	l_textureBinaryHeader.m_format     = l_texMetadata.format;
	l_textureBinaryHeader.m_dimension  = l_texMetadata.dimension;
	l_textureBinaryHeader.m_miscFlags  = l_texMetadata.miscFlags;
	l_textureBinaryHeader.m_miscFlags2 = l_texMetadata.miscFlags2;

	return l_textureBinaryHeader;
}

FWK::Converter::TextureBinaryConverter::TextureBinarySubresourceHeader FWK::Converter::TextureBinaryConverter::CreateTextureBinarySubresuourceHeader(const DirectX::Image& a_image) const
{
	TextureBinarySubresourceHeader l_textureBinarySubrourceHeader = {};

	l_textureBinarySubrourceHeader.m_width         = a_image.width;
	l_textureBinarySubrourceHeader.m_height        = a_image.height;
	l_textureBinarySubrourceHeader.m_rowPitch      = a_image.rowPitch;
	l_textureBinarySubrourceHeader.m_slicePitch    = a_image.slicePitch;
	l_textureBinarySubrourceHeader.m_pixelDataSize = a_image.slicePitch;

	return l_textureBinarySubrourceHeader;
}

std::uint64_t FWK::Converter::TextureBinaryConverter::CalculateTextureAssetFileSize(const DirectX::ScratchImage& a_scratchImage)
{
	auto l_textureAssetFileSize = CalculateBinaryDataSize<TextureBinaryHeader>(GetREFSingleBinaryElementCount());

	const auto* l_imageList = a_scratchImage.GetImages();

	if (!l_imageList) { return Constant::k_emptyAssetFileSize; }

	const auto& l_imageCount = a_scratchImage.GetImageCount();

	for (std::uint64_t l_imageIndex = 0ULL; l_imageIndex < l_imageCount; ++l_imageIndex)
	{
		const auto& l_image = l_imageList[l_imageIndex];

		l_textureAssetFileSize += CalculateBinaryDataSize<TextureBinarySubresourceHeader>(GetREFSingleBinaryElementCount());
		l_textureAssetFileSize += l_image.slicePitch;
	}

	return l_textureAssetFileSize;
}

bool FWK::Converter::TextureBinaryConverter::IsValidScratchImage(const DirectX::ScratchImage& a_scratchImage) const
{
	const auto& l_texMetadata = a_scratchImage.GetMetadata();

	if (!a_scratchImage.GetImages()) { return false; }
	if (!a_scratchImage.GetPixels()) { return false; }

	if (a_scratchImage.GetImageCount() == k_emptyTextureSubresourceCount) { return false; }
	if (a_scratchImage.GetPixelsSize() == k_emptyTexturePixelDataSize)    { return false; }

	if (l_texMetadata.width     == k_emptyTextureWidth)     { return false; }
	if (l_texMetadata.height    == k_emptyTextureHeight)    { return false; }
	if (l_texMetadata.depth     == k_emptyTextureDepth)     { return false; }
	if (l_texMetadata.arraySize == k_emptyTextureArraySize) { return false; }
	if (l_texMetadata.mipLevels == DXGI_FORMAT_UNKNOWN)		{ return false; }

	if (l_texMetadata.format == DXGI_FORMAT_UNKNOWN) { return false; }

	// CubeMap/3DTexture/TextureArrayは必要になったら読み込み対応をする
	if (l_texMetadata.dimension != DirectX::TEX_DIMENSION_TEXTURE2D) { return false; }

	return true;
}