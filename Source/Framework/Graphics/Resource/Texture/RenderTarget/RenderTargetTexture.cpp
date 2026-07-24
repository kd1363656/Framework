#include "RenderTargetTexture.h"

bool FWK::Graphics::RenderTargetTexture::Create(const Device&					          a_device,
												const GPUMemoryAllocator&		          a_gpuMemoryAllocator,
												const TypeAlias::Math::Color&		      a_clearColor,
												const DXGI_FORMAT					      a_format,
											    const UINT							      a_width,
											    const UINT							      a_height,
													  TypeAlias::RTVDescriptorPool&       a_rtvDescriptorPool,
													  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF(!Utility::IsValidTextureSize(a_width, a_height), "RenderTargetTextureのサイズがになっており、作成処理に失敗しました。",     false);
	FWK_ASSERT_RETURN_VALUE_IF(a_format == DXGI_FORMAT_UNKNOWN,                 "RenderTargetTextureのFormatが無効になっており、作成方法に失敗しました。", false);

	m_format     = a_format;
	m_clearColor = a_clearColor;

	FWK_ASSERT_RETURN_VALUE_IF(!CreateGPUResource(a_gpuMemoryAllocator, a_width, a_height), "RenderTargetTexture用GPUResourceの作成に失敗しており、作成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!CreateRTV(a_device, a_rtvDescriptorPool),                   "RenderTarget用RTVの作成に失敗しており、作成処理に失敗しました。",                false);
	FWK_ASSERT_RETURN_VALUE_IF(!CreateSRV(a_device, a_cbvSRVUAVDescriptorPool),             "RenderTarget用SRVの作成に失敗しており、作成処理に失敗しました。",                false);

	m_currentResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	return true;
}
bool FWK::Graphics::RenderTargetTexture::Resize(const Device&						      a_device,
												const GPUMemoryAllocator&			      a_gpuMemoryAllocator,
												const UINT64&						      a_retiredFenceValue,
												const UINT							      a_width,
											    const UINT							      a_height,
													  TypeAlias::RTVDescriptorPool&       a_rtvDescriptorPool, 
													  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
													  ResourceReleaseContext&	          a_resourceReleaseContext)
{
	// 同じサイズならリサイズ処理をする必要がないからreturn
	if (Utility::IsSameSize(a_width, 
							a_height,
							m_width,
							m_height))
	{
		return true; 
	}

	FWK_ASSERT_RETURN_VALUE_IF(!Utility::IsValidTextureSize(a_width, a_height), "RenderTargetTextureのリサイズ後のサイズが無効になっており、リサイズ処理に失敗しました。", false);

	RenderTargetTexture l_newRenderTargetTexture = {};

	FWK_ASSERT_RETURN_VALUE_IF(!l_newRenderTargetTexture.Create(a_device,
																a_gpuMemoryAllocator,
																m_clearColor,
																m_format,
																a_width,
																a_height,
																a_rtvDescriptorPool,
																a_cbvSRVUAVDescriptorPool),
																"リサイズ後のRenderTargetTextureの作成に失敗しており、リサイズ処理に失敗しました。",
																false);

	FWK_ASSERT_RETURN_VALUE_IF(!ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext), "古いRenderTargetTextureの遅延解放登録に失敗しており、リサイズ処理に失敗しました。", false);

	m_width  = a_width;
	m_height = a_height;

	*this = std::move(l_newRenderTargetTexture);

	return true;
}

bool FWK::Graphics::RenderTargetTexture::CreateGPUResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const UINT a_width, const UINT a_height)
{
	D3D12_CLEAR_VALUE l_clearValue = {};

	l_clearValue.Format = m_format;

	l_clearValue.Color[k_clearColorIndexR] = m_clearColor.R();
	l_clearValue.Color[k_clearColorIndexG] = m_clearColor.G();
	l_clearValue.Color[k_clearColorIndexB] = m_clearColor.B();
	l_clearValue.Color[k_clearColorIndexA] = m_clearColor.A();

	const auto l_resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_format,
															 a_width,
															 a_height,
															 Converter::TextureBinaryConverter::k_defaultTexture2DArraySize,
															 Converter::TextureBinaryConverter::k_defaultTexture2DMIPLevels,
															 Constant::k_defaultSampleCount,
															 Constant::k_defaultSampleQuality,
															 D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	FWK_ASSERT_RETURN_VALUE_IF(!a_gpuMemoryAllocator.CreateTextureResource(l_resourceDesc, 
																		   &l_clearValue, 
																		   k_defaultResourceState,
																		   m_gpuResource),
																		   "RenderTargetTexture用TextureResourceの作成に失敗しました。",
																		   false);

	m_width  = a_width;
	m_height = a_height;

	return true;
}
bool FWK::Graphics::RenderTargetTexture::CreateRTV(const Device& a_device, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF(!l_device,				  "デバイスが作成されておらず、RenderTargetTexture用のRTVの作成に失敗しました。",    false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_gpuResource.m_resource, "GPUResourceが作成されておらず、RenderTargetTexture用のRTVの作成に失敗しました。", false);

	const auto l_rtvDescriptorIndex = a_rtvDescriptorPool.Allocate();

	FWK_ASSERT_RETURN_VALUE_IF(l_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "RTVDescriptorIndexの確保に失敗しました。", false);

	// D3D12_RENDER_TARGET_VIEW_DESCについて
	// Format		 : RTVとしてみるときのフォーマット
	// ViewDimension : 2DTextureとしてRTVを作成する
	D3D12_RENDER_TARGET_VIEW_DESC l_rtvDesc = {};

	l_rtvDesc.Format		= m_format;
	l_rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	const auto& l_rtvHandle = a_rtvDescriptorPool.FetchVALCPUDescriptorHandle(l_rtvDescriptorIndex);

	// CreateRenderTargetView(RTVを作りたい対象リソース、
	//						  RTV設定、
	//						  RTVを書き込むCPUディスクリプタハンドル);
	l_device->CreateRenderTargetView(m_gpuResource.m_resource.Get(), &l_rtvDesc, l_rtvHandle);

	m_rtvDescriptorIndex = l_rtvDescriptorIndex;

	return true;
}
bool FWK::Graphics::RenderTargetTexture::CreateSRV(const Device& a_device, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF(!l_device,				  "デバイスが作成されておらず、RenderTargetTexture用のSRVの作成に失敗しました。",    false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_gpuResource.m_resource, "GPUResourceが作成されておらず、RenderTargetTexture用のSRVの作成に失敗しました。", false);

	const auto l_srvDescriptorIndex = a_cbvSRVUAVDescriptorPool.Allocate();

	FWK_ASSERT_RETURN_VALUE_IF(l_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "SRVDescriptorIndexの確保に失敗しました。", false);

	// D3D12_SHADER_RESOURCE_VIEW_DESCについて
	// Shader4ComponentMapping : Shader側でRGBA成分をどう読むか
	// Format                  : SRVとしてみるときのフォーマット
	// ViewDimension           : 2DTextureとしてSRVを作成する
	D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};

	l_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	l_srvDesc.Format				  = m_format;
	l_srvDesc.ViewDimension			  = D3D12_SRV_DIMENSION_TEXTURE2D;

	// D3D12_TEX2D_SRVについて
	// MostDetailedMIP     : 読み始めるMIP番号
	// MIPLevels	       : 読めるMIP数
	// PlaneSlice          : 通常カラーTextureなので0
	// ResourceMINLODCLAMP : 最小LOD制限
	l_srvDesc.Texture2D.MostDetailedMip     = k_mostDetailedMIP;
	l_srvDesc.Texture2D.MipLevels           = Converter::TextureBinaryConverter::k_defaultTexture2DMIPLevels;
	l_srvDesc.Texture2D.PlaneSlice		    = k_planeSlice;
	l_srvDesc.Texture2D.ResourceMinLODClamp = k_resourceMINLODClamp;

	const auto l_cpuHandle = a_cbvSRVUAVDescriptorPool.FetchVALCPUDescriptorHandle(l_srvDescriptorIndex);

	// CreateShaderResourceView(SRVを作りたい対象リソース、
	//							SRV設定、
	//							SRVを書き込むCPUディスクリプタハンドル);
	l_device->CreateShaderResourceView(m_gpuResource.m_resource.Get(), &l_srvDesc, l_cpuHandle);

	if (!a_cbvSRVUAVDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(a_device, l_srvDescriptorIndex))
	{
		a_cbvSRVUAVDescriptorPool.Release(l_srvDescriptorIndex);

		FWK_ASSERT_RETURN_VALUE("CPUOnlySRVからShaderVisibleSRVへのコピーに失敗したため、RenderTargetTexture用SRVの作成に失敗しました。", false);
	}

	m_srvDescriptorIndex = l_srvDescriptorIndex;

	return true;
}

bool FWK::Graphics::RenderTargetTexture::ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_gpuResource.m_resource,								         "RenderTargetTextureのGPUResourceが無効のため、遅延解放登録に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(m_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "RenderTargetTextureのRTVDescriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(m_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "RenderTargetTextureのSRVDescriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue  == Fence::k_unusedFenceValue,                "FenceValueが無効のため、RenderTargetTextureの遅延解放登録に失敗しました。",         false);

	ResourceReleaseContext::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

	l_gpuResourceReleaseRecord.m_gpuResource	   = std::move(m_gpuResource);
	l_gpuResourceReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	ResourceReleaseContext::DescriptorIndexReleaseRecord l_rtvDescriptorIndexReleaseRecord = {};

	l_rtvDescriptorIndexReleaseRecord.m_descriptorIndex	  = m_rtvDescriptorIndex;
	l_rtvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	ResourceReleaseContext::DescriptorIndexReleaseRecord l_srvDescriptorIndexReleaseRecord = {};

	l_srvDescriptorIndexReleaseRecord.m_descriptorIndex	  = m_srvDescriptorIndex;
	l_srvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord(std::move(l_gpuResourceReleaseRecord)),		       "RenderTargetTextureのGPUResourceの遅延解放登録に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseRTVDescriptorIndex(std::move(l_rtvDescriptorIndexReleaseRecord)),       "RenderTargetTextureのRTVDescriptorIndexの遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseCBVSRVUAVDescriptorIndex(std::move(l_srvDescriptorIndexReleaseRecord)), "RenderTargetTextureのSRVDescriptorIndexの遅延解放登録に失敗しました。", false);

	// 二重開放を防ぐため、DescriptorIndexは無効化する
	m_rtvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
	m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

	m_width  = Converter::TextureBinaryConverter::k_emptyTextureWidth;
	m_height = Converter::TextureBinaryConverter::k_emptyTextureHeight;

	m_currentResourceState = k_defaultResourceState;

	return true;
}