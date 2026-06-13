#include "RenderTargetTexture.h"

bool FWK::Graphics::RenderTargetTexture::Create(const Device&					    a_device,
												const GPUMemoryAllocator&		    a_gpuMemoryAllocator,
												const DXGI_FORMAT					a_format,
											    const UINT							a_width,
											    const UINT							a_height,
												const TypeAlias::Math::Color&		a_clearColor,
													  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool,
													  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!IsValidTextureSize(a_width, a_height), "RenderTargetTextureのサイズがになっており、作成処理に失敗しました。",     false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_format == DXGI_FORMAT_UNKNOWN,        "RenderTargetTextureのFormatが無効になっており、作成方法に失敗しました。", false);

	m_format     = a_format;
	m_clearColor = a_clearColor;

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateGPUResource(a_gpuMemoryAllocator, a_width, a_height), "RenderTargetTexture用GPUResourceの作成に失敗しており、作成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateRTV(a_device, a_rtvDescriptorPool),				       "RenderTarget用RTVの作成に失敗しており、作成処理に失敗しました。",				 false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateSRV(a_device, a_srvDescriptorPool),				       "RenderTarget用SRVの作成に失敗しており、作成処理に失敗しました。",				 false);

	m_currentResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	return true;
}
bool FWK::Graphics::RenderTargetTexture::Create(const Device&			            a_device, 
												const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
											    const UINT							a_width,
												const UINT							a_height,
													  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, 
													  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	return Create(a_device,
				  a_gpuMemoryAllocator,
				  Constant::k_defaultRenderTargetTextureFormat,
				  a_width,
				  a_height,
				  Constant::k_defaultBackBufferClearColor,
				  a_rtvDescriptorPool,
				  a_srvDescriptorPool);
}
bool FWK::Graphics::RenderTargetTexture::Resize(const Device&						a_device,
												const GPUMemoryAllocator&			a_gpuMemoryAllocator,
												const UINT64&						a_retiredFenceValue,
												const UINT							a_width,
											    const UINT							a_height,
													  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, 
													  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool, 
													  ResourceReleaseContext&	    a_resourceReleaseContext)
{
	// 同じサイズならリサイズ処理をする必要がないからreturn
	if (IsSameSize(a_width, a_height)) { return true; }

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!IsValidTextureSize(a_width, a_height), "RenderTargetTextureのリサイズ後のサイズが無効になっており、リサイズ処理に失敗しました。", false);

	RenderTargetTexture l_newRenderTargetTexture = {};

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_newRenderTargetTexture.Create(a_device,
																	   a_gpuMemoryAllocator,
																	   m_format,
																	   a_width,
																	   a_height,
																	   m_clearColor,
																	   a_rtvDescriptorPool,
																	   a_srvDescriptorPool),
																	   "リサイズ後のRenderTargetTextureの作成に失敗しており、リサイズ処理に失敗しました。",
																	   false);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext), "古いRenderTargetTextureの遅延解放登録に失敗しており、リサイズ処理に失敗しました。", false);

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
															 Constant::k_defaultTexture2DArraySize,
															 Constant::k_defaultTexture2DMipLevels,
															 Constant::k_defaultSampleCount,
															 Constant::k_defaultSampleQuality,
															 D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_gpuMemoryAllocator.CreateTextureResource(l_resourceDesc, 
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

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_device,					 "デバイスが作成されておらず、RenderTargetTexture用のRTVの作成に失敗しました。",    false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_gpuResource.m_resource, "GPUResourceが作成されておらず、RenderTargetTexture用のRTVの作成に失敗しました。", false);

	const auto l_rtvDescriptorIndex = a_rtvDescriptorPool.Allocate();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "RTVDescriptorIndexの確保に失敗しました。", false);

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
bool FWK::Graphics::RenderTargetTexture::CreateSRV(const Device& a_device, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_device,					 "デバイスが作成されておらず、RenderTargetTexture用のSRVの作成に失敗しました。",    false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_gpuResource.m_resource, "GPUResourceが作成されておらず、RenderTargetTexture用のSRVの作成に失敗しました。", false);

	const auto l_srvDescriptorIndex = a_srvDescriptorPool.Allocate();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_srvDescriptorIndex == Constant::k_invalidDescriptorIndex, "SRVDescriptorIndexの確保に失敗しました。", false);

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
	l_srvDesc.Texture2D.MipLevels           = Constant::k_defaultTexture2DMipLevels;
	l_srvDesc.Texture2D.PlaneSlice		    = k_planeSlice;
	l_srvDesc.Texture2D.ResourceMinLODClamp = k_resourceMINLODClamp;

	const auto l_cpuHandle = a_srvDescriptorPool.FetchVALCPUDescriptorHandle(l_srvDescriptorIndex);

	// CreateShaderResourceView(SRVを作りたい対象リソース、
	//							SRV設定、
	//							SRVを書き込むCPUディスクリプタハンドル);
	l_device->CreateShaderResourceView(m_gpuResource.m_resource.Get(), &l_srvDesc, l_cpuHandle);

	if (!a_srvDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(a_device, l_srvDescriptorIndex))
	{
		a_srvDescriptorPool.Release(l_srvDescriptorIndex);

		FWK_ASSERT_RETURN_VALUE("CPUOnlySRVからShadervisibleSRVへのコピーに失敗したため、RenderTargetTexture用SRVの作成に失敗しました。", false);
	}

	m_srvDescriptorIndex = l_srvDescriptorIndex;

	return true;
}

bool FWK::Graphics::RenderTargetTexture::ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_gpuResource.m_resource,									  "RenderTargetTextureのGPUResourceが無効のため、遅延解放登録に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex, "RenderTargetTextureのRTVDescriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_srvDescriptorIndex == Constant::k_invalidDescriptorIndex, "RenderTargetTextureのSRVDescriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_retiredFenceValue  == Constant::k_unusedFenceValue,		  "FenceValueが無効のため、RenderTargetTextureの遅延解放登録に失敗しました。",		   false);

	Struct::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

	l_gpuResourceReleaseRecord.m_gpuResource	   = std::move(m_gpuResource);
	l_gpuResourceReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	Struct::DescriptorIndexReleaseRecord l_rtvDescriptorIndexReleaseRecord = {};

	l_rtvDescriptorIndexReleaseRecord.m_descriptorIndex	  = m_rtvDescriptorIndex;
	l_rtvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	Struct::DescriptorIndexReleaseRecord l_srvDescriptorIndexReleaseRecord = {};

	l_srvDescriptorIndexReleaseRecord.m_descriptorIndex	  = m_srvDescriptorIndex;
	l_srvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord(std::move(l_gpuResourceReleaseRecord)),		    "RenderTargetTextureのGPUResourceの遅延解放登録に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseRTVDescriptorIndex(std::move(l_rtvDescriptorIndexReleaseRecord)), "RenderTargetTextureのRTVDescriptorIndexの遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseSRVDescriptorIndex(std::move(l_srvDescriptorIndexReleaseRecord)), "RenderTargetTextureのSRVDescriptorIndexの遅延解放登録に失敗しました。", false);

	// 二重開放を防ぐため、DescriptorIndexは無効化する
	m_rtvDescriptorIndex = Constant::k_invalidDescriptorIndex;
	m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;

	m_width  = Constant::k_emptyTextureWidth;
	m_height = Constant::k_emptyTextureHeight;

	m_currentResourceState = k_defaultResourceState;

	return true;
}

bool FWK::Graphics::RenderTargetTexture::IsValidTextureSize(const UINT a_width, const UINT a_height) const
{
	if (a_width  == Constant::k_emptyTextureWidth ||
		a_height == Constant::k_emptyTextureHeight) 
	{
		return false; 
	}
	
	return true;
}
bool FWK::Graphics::RenderTargetTexture::IsSameSize(const UINT a_width, const UINT a_height) const
{
	if (m_width  != a_width ||
		m_height != a_height)
	{
		return false;
	}

	return true;
}