#include "TextureBinaryConverter.h"

bool FWK::Converter::TextureBinaryConverter::LoadTextureAsset(const std::filesystem::path& a_filePath, DirectX::ScratchImage& a_scratchImage, DirectX::TexMetadata& a_texMetadata)
{
	// 読み込める.assetかどうかを確認する
	// 読み込めなければ.pngから再生成する
	if (!CanLoadTextureAsset(a_filePath)) { return false; }

	const auto& l_textureAssetFilePath = CreateTextureAssetFilePath(a_filePath);

	// .assetを読み込み専用のMemoryMappedFileとして開く
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateReadMemoryMappedFile(l_textureAssetFilePath), "TextureAssetの読み込み用MemoryMappedFile作成に失敗しており。バイナリーファイルの読み込みに失敗しました", false);

	// MemoryMappedFileの先頭アドレスを取得する
	const auto* l_mappedData = GetPTRMappedData();

	if (!l_mappedData)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetの読み込み元メモリが無効になっており、バイナリーファイルの読み込みに失敗しました。", false);
	}

	// 現在の読み込み位置、ファイルの先頭なので0からスタート
	auto l_memoryReadOffset = GetREFInitialMemoryReadOffset();

	TextureBinaryHeader l_textureBinaryHeader = {};

	// Header分のバイト数を安全に読めるかを確認する
	if (GetREFMappedDataSize() < sizeof(TextureBinaryHeader))
	{
#if defined(_DEBUG)
		const auto& l_debugLog = std::format("TextureAssetのファイルサイズがHeaderサイズよりも小さいため、PNGから再生成します。AssetFileSize : {}, HeaderSize : {}\n", GetREFMappedDataSize(), sizeof(TextureBinaryHeader));

		OutputDebugStringA(l_debugLog.c_str());
#endif
		DestroyMemoryMappedFile();

		return false;
	}

	// .asset先頭からTextureBinaryHeaderを読む
	ReadBinaryData(GetREFSingleBinaryElementCount(),	
				   l_mappedData,
				   l_memoryReadOffset,	
				   &l_textureBinaryHeader);

	// Texture用.assetかどうか確認する
	if (l_textureBinaryHeader.m_assetTypeID != k_textureAssetTypeID)
	{
#if defined(_DEBUG)
		const auto& l_debugLog = std::format("TextureAssetのTypeIDが一致しないため、PNGから再生成します。AssetTypeID : {}, CurrentAssetTypeID : {}\n", l_textureBinaryHeader.m_assetTypeID, k_textureAssetTypeID);

		OutputDebugStringA(l_debugLog.c_str());
#endif
		DestroyMemoryMappedFile();

		return false;
	}

	// 保存形式のバージョンが違うなら、古い.assetとして扱いPNGから再生成する
	if (l_textureBinaryHeader.m_version != k_textureAssetVersion)
	{
#if defined(_DEBUG)
		const auto& l_debugLog = std::format("TextureAssetのVersionが一致しないため、PNGから再生成します。AssetVersion : {}, CurrentAssetVersion : {}\n", l_textureBinaryHeader.m_version, k_textureAssetVersion);

		OutputDebugStringA(l_debugLog.c_str());
#endif
		DestroyMemoryMappedFile();

		return false;
	}

	// Headerに保存されたファイルサイズと、実際のファイルサイズが違うなら壊れている。
	if (l_textureBinaryHeader.m_fileSize != GetREFMappedDataSize())
	{
#if defined(_DEBUG)
		const auto& l_debugLog = std::format("TextureAssetのファイルサイズが一致しないため、PNGから再生成します。AssetFileSize : {}, CurrentFileSize : {}\n", l_textureBinaryHeader.m_fileSize, GetREFMappedDataSize());

		OutputDebugStringA(l_debugLog.c_str());
#endif
		DestroyMemoryMappedFile();

		return false;
	}

	// HeaderからDirectXTex用のTexMetadataを復元する
	a_texMetadata = CreateTexMetadata(l_textureBinaryHeader);

	// TexMetadataを使ってScratchImageの内部メモリを確保する。
	// この時点では「画像を入れる箱」を作るだけで、実際のピクセル値はまだ入っていない
	const auto l_hr = a_scratchImage.Initialize(a_texMetadata);

	if (FAILED(l_hr))
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetからScratchImageの初期化に失敗しており、バイナリーファイルの読み込みに失敗しました。", false);
	}

	// ScratchImage内のImage配列を取得する
	const auto* l_imageList = a_scratchImage.GetImages();

	if (!l_imageList)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetの読み込み先Image配列が無効となっており、バイナリーファイルの読み込みに失敗しました。", false);
	}

	// Headerに保存されていたSubresource数と、
	// ScratchImageがTexMetadataから作ったImages数が一致するか計算する
	if (a_scratchImage.GetImageCount() != l_textureBinaryHeader.m_subresourceCount)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetのサブリソース数がScratchImageと一致しておらず、バイナリーファイルの読み込みに失敗しました。", false);
	}

	// 各サブリソースを順番に読み込む
	for (std::uint64_t l_imageIndex = 0ULL; l_imageIndex < l_textureBinaryHeader.m_subresourceCount; ++l_imageIndex)
	{
		// SubresourceHeaderを読む前に、ファイル範囲内かだけ確認する
		if (l_memoryReadOffset > GetREFMappedDataSize() ||
			sizeof(TextureBinarySubresourceHeader) > GetREFMappedDataSize() - l_memoryReadOffset)
		{
			DestroyMemoryMappedFile();
			FWK_ASSERT_RETURN_VALUE("TextureAssetSubresourceHeaderを読み込めるサイズではないため、バイナリーファイルの読み込みに失敗しました。", false);
		}

		TextureBinarySubresourceHeader l_textureBinarySubresourceHeader = {};

		// MipMap1枚分の情報を読む
		ReadBinaryData(GetREFSingleBinaryElementCount(),
					   l_mappedData,
					   l_memoryReadOffset,
					   &l_textureBinarySubresourceHeader);

		const auto& l_image = l_imageList[l_imageIndex];

		// pixelDataSizeがScratchIMage側のSlicePitchと違う場合は
		// コピーするとメモリ破壊する可能性があるため、確認する
		if (l_textureBinarySubresourceHeader.m_pixelDataSize != l_image.slicePitch)
		{
			DestroyMemoryMappedFile();
			FWK_ASSERT_RETURN_VALUE("TextureAssetのPixelDataSizeがScratchImageと一致しておらず、バイナリーファイルの読み込みに失敗しました。", false);
		}

		// .assetに保存されていた幅と、ScratchImage側で作られたImageの幅が一致するか確認する
		if (l_textureBinarySubresourceHeader.m_width != l_image.width)
		{
			DestroyMemoryMappedFile();
			FWK_ASSERT_RETURN_VALUE("TextureAssetの幅がScratchImageと一致しておらず、バイナリーファイルの読み込みに失敗しました。", false);
		}

		// 高さ確認
		if (l_textureBinarySubresourceHeader.m_height != l_image.height)
		{
			DestroyMemoryMappedFile();
			FWK_ASSERT_RETURN_VALUE("TextureAssetの高さがScratchImageと一致しておらず、バイナリーファイルの読み込みに失敗しました。", false);
		}

		// rowPitch確認
		// rowPitchは1行分のバイト数。
		// フォーマットや幅違うと一致しなくなる
		if (l_textureBinarySubresourceHeader.m_rowPitch != l_image.rowPitch)
		{
			DestroyMemoryMappedFile();
			FWK_ASSERT_RETURN_VALUE("TextureAssetのRowPitchがScratchImageと一致しておらず、バイナリーファイルの読み込みに失敗しました。", false);
		}

		// SlicePitch確認
		// SlicePitchはImage全体のバイト数
		if (l_textureBinarySubresourceHeader.m_slicePitch != l_image.slicePitch)
		{
			DestroyMemoryMappedFile();
			FWK_ASSERT_RETURN_VALUE("TextureAssetのSlicePitchがScratchImageと一致しておらず、バイナリーファイルの読み込みに失敗しました。", false);
		}
		// .assert先頭からTextureBinaryHeaderを読む
		// ピクセルデータ本体を読めるだけの残りサイズがあるか確認する
		if (l_memoryReadOffset > GetREFMappedDataSize() ||
			l_textureBinarySubresourceHeader.m_pixelDataSize > GetREFMappedDataSize() - l_memoryReadOffset)
		{
			DestroyMemoryMappedFile();
			FWK_ASSERT_RETURN_VALUE("TextureAssetのピクセルデータを読み込めるサイズでないため、バイナリーファイルの読み込みに失敗しました。。", false);
		}

		// .asset内のピクセルデータを、ScratchImageが確保した画像メモリへコピーする
		ReadBinaryData(l_textureBinarySubresourceHeader.m_pixelDataSize,
					   l_mappedData,
					   l_memoryReadOffset,
					   l_image.pixels);
	}

	// 全Subresourceを読み終わった後に、最後の読み込み位置を確認する
	if (l_memoryReadOffset != l_textureBinaryHeader.m_fileSize)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetの読み込みサイズがヘッダーのファイルサイズと一致しておらず、バイナリーファイルの読み込みに失敗しました。", false);
	}

	DestroyMemoryMappedFile();

	return true;
}

bool FWK::Converter::TextureBinaryConverter::SaveTextureAsset(const std::filesystem::path& a_filePath, const DirectX::ScratchImage& a_scratchImage)
{
	// 保存元のPNGが存在するかどうか、DirectXTexで読み込んだScratchImageが正常かを確認する
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension), "TextureAssetの元になるPNGファイルが無効となっており、バイナリーファイルの保存に失敗しました。", false);

	// PNGと同じ場所・同じ名前で拡張子だけ.assetにしたパスを作る
	const auto& l_textureAssetFilePath = CreateTextureAssetFilePath   (a_filePath);
	const auto& l_textureAssetFileSize = CalculateTextureAssetFileSize(a_scratchImage);

	// 書き込み用メモリマップドファイルの作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_textureAssetFileSize == Constant::k_emptyAssetFileSize,	                    "TextureAssetへ保持するScratchImageが無効となっており、バイナリーファイルの保存に失敗しました。",	    false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateWriteMemoryMappedFile(l_textureAssetFilePath, l_textureAssetFileSize), "TextureAssetの書き込み用MemoryMappedFile作成に失敗ており、バイナリーファイルの保存に失敗しました。。", false);

	// 作成したメモリマップマップドデータを取得
	auto* l_mappedData = GetMutablePTRMappedData();

	if (!l_mappedData)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetの書き込み先メモリが無効となっており、バイナリーファイルの保存に失敗しました。", false);
	}

	auto l_memoryWriteOffset = GetREFInitialMemoryWriteOffset();

	// メモリマップドファイル書き込み用テクスチャバイナリヘッダーを作成
	const auto& l_textureBinaryHeader = CreateTextureBinaryHeader(a_scratchImage, l_textureAssetFileSize);

	// Headerを書き込む、
	// WriteBinaryData内で、書き込んだ分だけl_memoryWriteOffsetが進む
	WriteBinaryData(GetREFSingleBinaryElementCount(),
					&l_textureBinaryHeader,
					l_memoryWriteOffset,
					l_mappedData);

	// ScratchImageからイメージリストを取得
	const auto* l_imageList = a_scratchImage.GetImages();

	if (!l_imageList)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetへ保存するImage配列が無効となっており、バイナリーファイルの保存に失敗しました。", false);
	}

	const auto l_imageCount = a_scratchImage.GetImageCount();

	for (std::uint64_t l_imageIndex = 0ULL; l_imageIndex < l_imageCount; ++l_imageIndex)
	{
		// テクスチャイメージからサブリソースヘッダーを作成し書き込む
		const auto& l_image							 = l_imageList[l_imageIndex];
		const auto& l_textureBinarySubresourceHeader = CreateTextureBinarySubresourceHeader(l_image);

		// SubresourceHeaderを書き込む
		WriteBinaryData(GetREFSingleBinaryElementCount(),
						&l_textureBinarySubresourceHeader,
						l_memoryWriteOffset,
						l_mappedData);

		// ピクセルデータ本体を書き込む
		WriteBinaryData(l_textureBinarySubresourceHeader.m_pixelDataSize,
						l_image.pixels,
						l_memoryWriteOffset,
						l_mappedData);
	}

	// 計算したファイルサイズと、実際に書き込んだサイズが一致するかを確認する
	if (l_memoryWriteOffset != l_textureAssetFileSize)
	{
		DestroyMemoryMappedFile();
		FWK_ASSERT_RETURN_VALUE("TextureAssetの書き込みサイズが計算したファイルサイズと一致せず、バイナリーファイルの保存に失敗しました。", false);
	}

	DestroyMemoryMappedFile();

	return true;
}

bool FWK::Converter::TextureBinaryConverter::CanLoadTextureAsset(const std::filesystem::path& a_filePath) const
{
	// まず元のPNGが存在していて、拡張子も.pngか確認する
	if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension)) { return false; }

	// PNGと同じ場所・同じ名前で拡張子だけ.assetに変えたパスを作成する
	const auto& l_textureAssetFilePath = CreateTextureAssetFilePath(a_filePath);

	std::error_code l_sourceErrorCode = {};
	std::error_code l_assetErrorCode  = {};

	// PNGと.assetの最終更新時刻を取得する。
	// これにより「PNGを更新したのに古い.assetを読んでしまう」問題を防ぐ。
	const auto l_sourceLastWriteTime = std::filesystem::last_write_time(a_filePath,			    l_sourceErrorCode);
	const auto l_assetLastWriteTime  = std::filesystem::last_write_time(l_textureAssetFilePath, l_assetErrorCode);

	if (l_sourceErrorCode) { return false; }
	if (l_assetErrorCode)  { return false; }

	// .assetがPNGより古い場合は、PNGの内容が更新されている可能性がある。
	// そのため、.assetがPNGと同じか新しい場合だけキャッシュとして扱う
	return l_assetLastWriteTime >= l_sourceLastWriteTime;
}

std::filesystem::path FWK::Converter::TextureBinaryConverter::CreateTextureAssetFilePath(const std::filesystem::path& a_filePath) const
{
	return Utility::CreateFilePathByReplaceExtension(a_filePath, Constant::k_lowerAssetExtension);
}

FWK::Converter::TextureBinaryConverter::TextureBinaryHeader FWK::Converter::TextureBinaryConverter::CreateTextureBinaryHeader(const DirectX::ScratchImage& a_scratchImage, const std::uint64_t& a_fileSize) const
{
	TextureBinaryHeader l_textureBinaryHeader = {};

	const auto& l_texMetadata = a_scratchImage.GetMetadata();

	l_textureBinaryHeader.m_fileSize    = a_fileSize;
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

FWK::Converter::TextureBinaryConverter::TextureBinarySubresourceHeader FWK::Converter::TextureBinaryConverter::CreateTextureBinarySubresourceHeader(const DirectX::Image& a_image) const
{
	TextureBinarySubresourceHeader l_textureBinarySubresourceHeader = {};

	l_textureBinarySubresourceHeader.m_width         = a_image.width;
	l_textureBinarySubresourceHeader.m_height        = a_image.height;
	l_textureBinarySubresourceHeader.m_rowPitch      = a_image.rowPitch;
	l_textureBinarySubresourceHeader.m_slicePitch    = a_image.slicePitch;
	l_textureBinarySubresourceHeader.m_pixelDataSize = a_image.slicePitch;

	return l_textureBinarySubresourceHeader;
}

DirectX::TexMetadata FWK::Converter::TextureBinaryConverter::CreateTexMetadata(const TextureBinaryHeader& a_textureBinaryHeader) const
{
	DirectX::TexMetadata l_texMetadata = {};

	l_texMetadata.width      = a_textureBinaryHeader.m_width;
	l_texMetadata.height     = a_textureBinaryHeader.m_height;
	l_texMetadata.depth      = a_textureBinaryHeader.m_depth;
	l_texMetadata.arraySize  = a_textureBinaryHeader.m_arraySize;
	l_texMetadata.mipLevels  = a_textureBinaryHeader.m_mipLevels;
	l_texMetadata.format     = static_cast<DXGI_FORMAT>(a_textureBinaryHeader.m_format);
	l_texMetadata.dimension  = static_cast<DirectX::TEX_DIMENSION>(a_textureBinaryHeader.m_dimension);
	l_texMetadata.miscFlags  = a_textureBinaryHeader.m_miscFlags;
	l_texMetadata.miscFlags2 = a_textureBinaryHeader.m_miscFlags2;

	return l_texMetadata;
}

std::uint64_t FWK::Converter::TextureBinaryConverter::CalculateTextureAssetFileSize(const DirectX::ScratchImage& a_scratchImage) const
{
	// TextureBinaryHeader1個分のサイズを足す
	auto l_textureAssetFileSize = CalculateBinaryDataSize<TextureBinaryHeader>(GetREFSingleBinaryElementCount());

	const auto* l_imageList = a_scratchImage.GetImages();

	if (!l_imageList) { return Constant::k_emptyAssetFileSize; }

	const auto& l_imageCount = a_scratchImage.GetImageCount();

	if (l_imageCount == k_emptyTextureSubresourceCount) { return Constant::k_emptyAssetFileSize; }

	for (std::uint64_t l_imageIndex = 0ULL; l_imageIndex < l_imageCount; ++l_imageIndex)
	{
		const auto& l_image = l_imageList[l_imageIndex];

		// 各ImageごとにSubresourceHeader1個分のサイズを足す
		l_textureAssetFileSize += CalculateBinaryDataSize<TextureBinarySubresourceHeader>(GetREFSingleBinaryElementCount());
		l_textureAssetFileSize += l_image.slicePitch;
	}

	return l_textureAssetFileSize;
}