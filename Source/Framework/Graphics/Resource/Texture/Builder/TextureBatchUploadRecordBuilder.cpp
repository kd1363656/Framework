#include "TextureBatchUploadRecordBuilder.h"

bool FWK::Graphics::TextureBatchUploadRecordBuilder::CreateTextureBatchUploadRecord(const Device&							  a_device, 
																					const GPUMemoryAllocator&				  a_gpuMemoryAllocator, 
																					const std::wstring&						  a_filePath,
																					const DirectX::ScratchImage&			  a_scratchImage, 
																				    const DirectX::TexMetadata&				  a_texMetadata, 
																					const TypeAlias::StorageID				  a_storageID, 
																						  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
																						  Struct::TextureBatchUploadRecord&   a_textureBatchUploadRecord) const
{
	auto& l_textureRecord = a_textureBatchUploadRecord.m_textureRecord;

	// もしTextureRecordがインスタンス化されていなければ
	// インスタンス化する
	if (!l_textureRecord)
	{
		l_textureRecord = std::make_shared<Graphics::TextureRecord>();
	}

	// まずはGPU側用のテクスチャリソースのヒープ領域を確保
	FWK_ASSERT_RETURN_VALUE_IF(!CreateTextureResource(a_gpuMemoryAllocator, a_texMetadata, *l_textureRecord), "TextureResource作成処理に失敗したため、テクスチャバッチアップロード情報作成処理に失敗しました。", false);

	// ScratchImageの画像データをUploadBufferへ書き込み、UploadSystemへ渡すコピー情報を作成する
	// ここではCopyCommandQueueへ送信しない(バッチ処理を行うため)
	FWK_ASSERT_RETURN_VALUE_IF(!CreateTextureUploadRecord(a_device, a_scratchImage, a_textureBatchUploadRecord), "テクスチャサブリソースアップロード情報作成処理に失敗しました。", false);

	// 作成したTextureResourceをシェーダーから参照できるように、CPUOnly側のDescriptorHeapへSRVを作成する
	FWK_ASSERT_RETURN_VALUE_IF(!CreateTextureSRV(a_device,
												 a_texMetadata,
												 a_cbvSRVUAVDescriptorPool,
												 *a_textureBatchUploadRecord.m_textureRecord),
												 "TextureSRV作成に失敗したため、テクスチャアップロード情報作成処理に失敗しました。",
												 false);

	// CPUOnlyに作成したSRVをShaderVisible側へコピーする
	FWK_ASSERT_RETURN_VALUE_IF(!a_cbvSRVUAVDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(a_device, l_textureRecord->GetVALSRVDescriptorIndex()), "CPUOnlyからShaderVisibleSRVへのコピーに失敗したため、TextureSRV作成処理に失敗しました。", false);

	l_textureRecord->SetReferenceCount(AssetRecordBase::k_defaultAssetReferenceCount);
	l_textureRecord->SetStorageID     (a_storageID);
	l_textureRecord->SetFilePath	  (a_filePath);

	return true;
}

bool FWK::Graphics::TextureBatchUploadRecordBuilder::CreateTextureResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const DirectX::TexMetadata& a_texMetadata, Graphics::TextureRecord& a_textureRecord) const
{
	FWK_ASSERT_RETURN_VALUE_IF(a_texMetadata.format	  == DXGI_FORMAT_UNKNOWN,			   "テクスチャフォーマットが無効のため、TextureResource作成処理に失敗しました。",                  false);
	FWK_ASSERT_RETURN_VALUE_IF(a_texMetadata.dimension != DirectX::TEX_DIMENSION_TEXTURE2D, "TextureResource作成処理はTexture2Dのみ対応しており、TextureResource作成処理に失敗しました。", false);

	ResourceReleaseContext::GPUResource l_gpuResource = {};

	// Texture2D用のD3D12_RESOURCE_DESCを作成する
	// Tex2D(フォーマット、
	//		 横幅、
	//		 縦幅、
	//		 配列数、
	//		 MIP数);
	const auto l_textureResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(a_texMetadata.format,
																	a_texMetadata.width,
																	static_cast<UINT>(a_texMetadata.height),
																	static_cast<UINT16>(a_texMetadata.arraySize),
																	static_cast<UINT16>(a_texMetadata.mipLevels));

	// CopyCommandQueueでCopyTextureRegionのコピー先として使うため、
	// CopyCommandQueueが要求するD3D12_RESOURCE_STATE_COMMONで作成する
	FWK_ASSERT_RETURN_VALUE_IF(!a_gpuMemoryAllocator.CreateTextureResource(l_textureResourceDesc,
																		   nullptr,
																		   D3D12_RESOURCE_STATE_COMMON,
																		   l_gpuResource),
																		   "D3D12MAによるTextureResource作成処理に失敗しており、TextureResource作成処理に失敗しました。",
																		   false);

	a_textureRecord.SetGPUResource(std::move(l_gpuResource));

	return true;
}

bool FWK::Graphics::TextureBatchUploadRecordBuilder::CreateTextureUploadRecord(const Device& a_device, const DirectX::ScratchImage& a_scratchImage, Struct::TextureBatchUploadRecord& a_textureBatchUploadRecord) const
{
	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスが作成されておらず、テクスチャサブリソースアップロード情報作成処理に失敗しました。", false);

	const auto& l_textureRecord = a_textureBatchUploadRecord.m_textureRecord;

	FWK_ASSERT_RETURN_VALUE_IF(!l_textureRecord, "TextureRecordがインスタンス化されておらず、テクスチャサブリソースアップロード情報作成処理に失敗しました。", false);

	const auto& l_textureResource = l_textureRecord->GetREFGPUResource().m_resource;

	FWK_ASSERT_RETURN_VALUE_IF(!l_textureResource, "TextureResourceが作成されておらず、テクスチャサブリソースアップロード情報作成処理に失敗しました。", false);
	
	const auto& l_textureResourceDesc = l_textureResource->GetDesc();
	const auto  l_subresourceCount    = static_cast<UINT>         (a_scratchImage.GetImageCount());

	auto& l_textureUploadRecord = a_textureBatchUploadRecord.m_textureUploadRecord;

	// 各サブリソースをUploadBufferの何バイト目からどんな形で置けばいいかを覚えておくリスト
	auto& l_layoutList = l_textureUploadRecord.m_layoutList;

	// 念のため要素をclear()してからサブリソース分の容量を確保
	l_layoutList.clear ();
	l_layoutList.resize(static_cast<size_t>(l_subresourceCount));

	// 各サブリソースの行数
	auto l_rowCountList = std::vector<UINT>(l_subresourceCount);

	// 1行当たりの実データサイズ
	auto l_rowSizeInBytesList = std::vector<UINT64>(l_subresourceCount);

	auto l_requiredUploadBufferSize = k_initialRequiredUploadBufferSize;

	// TextureResourceへコピーするために必要なUploadBuffer上の配置情報を計算する
	// GetCopyableFootprints(コピー先TextureResourceの設定、
	//						 計算を開始するサブリソース番号、
	//						 計算を開始するサブリソース数、
	//						 UploadBuffer内の配置情報計算開始オフセット、
	//						 サブリソースごとのUploadBuffer配置情報の受取先、
	//						 サブリソースごとの行数の受取先、
	//						 サブリソースごとの1行当たりの有効データサイズの受取先
	//						 必要なUploadBufferサイズの受取先);
	l_device->GetCopyableFootprints(&l_textureResourceDesc,
									UploadBuffer::k_firstSubresourceIndex,
									l_subresourceCount,
									k_uploadBufferBeginOffset,
									l_layoutList.data(),
									l_rowCountList.data(),
									l_rowSizeInBytesList.data(),
									&l_requiredUploadBufferSize);

	auto& l_uploadBuffer = l_textureUploadRecord.m_uploadBuffer;

	// DEFAULTヒープ上にあるTextureResourceは直接CPUから書き込むことはできないため
	// CPU書き込み可能なUploadBufferを作成する
	FWK_ASSERT_RETURN_VALUE_IF(!l_uploadBuffer.Create(a_device, l_requiredUploadBufferSize), "テクスチャ用UploadBufferの作成処理に失敗したため、テクスチャサブリソースアップロード情報作成処理に失敗しました。", false);

	auto* l_mappedData = l_uploadBuffer.FetchPTRMappedData();

	FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "UploadBufferのMapに失敗したため、テクスチャサブリソースアップロード情報作成処理に失敗しました。", false);

	const auto* l_imageList = a_scratchImage.GetImages();

	FWK_ASSERT_RETURN_VALUE_IF(!l_imageList, "ScratchImageの画像データ取得に失敗したため、テクスチャサブリソースアップロード情報作成処理に失敗しました。", false);

	// DirectXTexで読み込んだ画像データをUploadBufferへコピーする
	// Texture2Dでは各MipMapが一つのサブリソースになる(Texture2DArrayなどでは違う)
	for (UINT l_subresourceIndex = 0U; l_subresourceIndex < l_subresourceCount; ++l_subresourceIndex)
	{
		// 現在処理するサブリソースの元画像データと、UploadBuffer上の配置情報を取得する
		const auto& l_image  = l_imageList [l_subresourceIndex];
		const auto& l_layout = l_layoutList[l_subresourceIndex];

		// 現在のサブリソースを書き込むUploadBuffer上の先頭アドレスを取得する
		// l_layout.OffsetはGetCopyableFootprints()が計算したサブリソースごとの開始位置
		auto* l_destinationSubresource = l_mappedData + l_layout.Offset;

		// UploadBuffer側で1行進むためのバイト数を取得する
		// RowPitchにはD3D12のコピー条件に合わせてアライメントされた値になる
		const auto l_destinationRowPitch = l_layout.Footprint.RowPitch;

		// UploadBuffer側でDepth方向に1枚進むためのバイト数を計算する
		// Texture2DではDepthは基本1だが、計算式としては1枚分 = RowPitch * コピーするサブリソースの行数
		const auto& l_destinationSlicePitch = l_destinationRowPitch * static_cast<std::size_t>(l_rowCountList[l_subresourceIndex]);

		// ScratchImage側で1行進むためのバイト数を取得する
		const auto& l_sourceRowPitch = l_image.rowPitch;

		// ScratchImage側でDepth方向に1枚進むためのバイト数を取得する
		const auto& l_sourceSlicePitch = l_image.slicePitch;

		// 実際にコピーする1行当たりの有効データサイズ
		// UploadBuffer側のRowPitch全体ではなく、有効な画像データ部分だけコピーする
		// (RowPitchでアライメントするための余白を含んでいないということ)
		const auto l_copyRowSize = l_rowSizeInBytesList[l_subresourceIndex];

		// Depth方向にコピーする
		for (UINT l_depthIndex = 0U; l_depthIndex < l_layout.Footprint.Depth; ++l_depthIndex)
		{
			// RowPitchがコピー元とコピー先で異なる可能性があるため、1行ずつコピーする
			for (UINT l_rowIndex = 0U; l_rowIndex < l_rowCountList[l_subresourceIndex]; ++l_rowIndex)
			{
				// UploadBuffer側の現在の行の書き込み先アドレスを計算する
				auto* l_destination = l_destinationSubresource + l_depthIndex * l_destinationSlicePitch + l_rowIndex * l_destinationRowPitch;

				// ScratchImage側の現在の行の読み取り元アドレスを計算する
				const auto* l_source = l_image.pixels + l_depthIndex * l_sourceSlicePitch + l_rowIndex * l_sourceRowPitch;

				// ScratchImage側の1行分の有効データをUploadBuffer側の対応する行へコピーする
				std::memcpy(l_destination, l_source, l_copyRowSize);
			}
		}
	}

	return true;
}

bool FWK::Graphics::TextureBatchUploadRecordBuilder::CreateTextureSRV(const Device&				                a_device, 
																	  const DirectX::TexMetadata&               a_texMetadata, 
																		    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
																			Graphics::TextureRecord&	        a_textureRecord) const
{
	const auto& l_textureResource = a_textureRecord.GetREFGPUResource().m_resource;

	FWK_ASSERT_RETURN_VALUE_IF(!l_textureResource,						  "TextureResourceが無効のため、TextureSRV作成処理に失敗しました。",          false);
	FWK_ASSERT_RETURN_VALUE_IF(a_texMetadata.format == DXGI_FORMAT_UNKNOWN, "テクスチャフォーマットが無効のため、TextureSRV作成処理に失敗しました。", false);

	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスが作成されておらず、TextureSRV作成処理に失敗しました。", false);

	// D3D12_SHADER_RESOURCE_VIEW_DESCについて
	// Shader4ComponentMapping : Shader側でRGBA部分をどのように読むか
	// Format                  : Shader側から読むときのTextureFormat
	// ViewDimension		   : Shader側から見たリソースの種類
	D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};

	l_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	l_srvDesc.Format				  = a_texMetadata.format;

	if (a_texMetadata.arraySize >= k_texture2DArrayMINArraySize)
	{
		l_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

		// D3D12_TEX2D_ARRAY_SRVについて
		// MostDetailedMIP     : 読み始めるMIP番号
		// MIPLevels	       : 読めるMIP数
		// FirstArraySlice     : Shaderから読み始めるArraySlice番号
		// ArraySize		   : Shaderから読めるArraySlice数。 
		// PlaneSlice          : 参照するPlane番号
		// ResourceMINLODCLAMP : 最小LOD制限
		l_srvDesc.Texture2DArray.MostDetailedMip     = k_mostDetailedMIP;
		l_srvDesc.Texture2DArray.MipLevels           = static_cast<UINT>(a_texMetadata.mipLevels);
		l_srvDesc.Texture2DArray.FirstArraySlice     = k_firstArraySlice;
		l_srvDesc.Texture2DArray.ArraySize           = static_cast<UINT>(a_texMetadata.arraySize);
		l_srvDesc.Texture2DArray.PlaneSlice			 = k_planeSlice;
		l_srvDesc.Texture2DArray.ResourceMinLODClamp = k_resourceMINLODClamp;
	}
	else
	{
		l_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		// D3D12_TEX2D_SRVについて
		// MostDetailedMIP     : 読み始めるMIP番号
		// MIPLevels	       : 読めるMIP数
		// PlaneSlice          : 参照するPlane番号
		// ResourceMINLODCLAMP : 最小LOD制限
		l_srvDesc.Texture2D.MostDetailedMip     = k_mostDetailedMIP;
		l_srvDesc.Texture2D.MipLevels           = static_cast<UINT>(a_texMetadata.mipLevels);
		l_srvDesc.Texture2D.PlaneSlice		    = k_planeSlice;
		l_srvDesc.Texture2D.ResourceMinLODClamp = k_resourceMINLODClamp;
	}

	// SRVストレージIDを格納
	const auto l_srvDescriptorIndex = a_cbvSRVUAVDescriptorPool.Allocate();

	FWK_ASSERT_RETURN_VALUE_IF(l_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "SRV用ストレージIDの確保に失敗したため、TextureSRV作成処理に失敗しました。", false);

	a_textureRecord.SetSRVDescriptorIndex(l_srvDescriptorIndex);

	const auto l_cpuOnlyCPUHandle = a_cbvSRVUAVDescriptorPool.FetchVALCPUDescriptorHandle(a_textureRecord.GetVALSRVDescriptorIndex());

	// 作成したビューを用いてTextureResourceとSRVを結び付ける
	// CreateShaderResourceView(SRVとして参照したいGPUResource、
	//						    SRVの設定、Texture2D/Texture2DArray/Format/MIP数など
	//							SRVを書き込むCPU側DescriptorHandle);
	l_device->CreateShaderResourceView(l_textureResource.Get(), &l_srvDesc, l_cpuOnlyCPUHandle);

	return true;
}