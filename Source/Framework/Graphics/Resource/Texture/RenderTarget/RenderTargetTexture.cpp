#include "RenderTargetTexture.h"

bool FWK::Graphics::RenderTargetTexture::Create(const Device&			            a_device, 
												const GPUMemoryAllocator&           a_gpuMemoryAllocator,
											    const Struct::ClientSize&           a_clientSize, 
													  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, 
													  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!IsValidClientSize(a_clientSize), "RenderTargetTextureのサイズがになっており、作成処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateGPUResource(a_gpuMemoryAllocator, a_clientSize), "RenderTargetTexture用GPUResourceの作成に失敗しており、作成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateRTV(a_device, a_rtvDescriptorPool),				  "RenderTarget用RTVの作成に失敗しており、作成処理に失敗しました。",				false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateSRV(a_device, a_srvDescriptorPool),				  "RenderTarget用SRVの作成に失敗しており、作成処理に失敗しました。",				false);

	m_currentResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	return true;
}
bool FWK::Graphics::RenderTargetTexture::Resize(const Device&						a_device,
												const GPUMemoryAllocator&			a_gpuMemoryAllocator,
												const Struct::ClientSize&			a_clientSize,
												const UINT64&						a_retiredFenceValue,
													  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, 
													  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool, 
													  ResourceReleaseContext&	    a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!IsValidClientSize(a_clientSize), "RenderTargetTextureのリサイズ後のサイズが無効になっており、リサイズ処理に失敗しました。", false);

	if (IsSameSize(a_clientSize)) { return true; }

	RenderTargetTexture l_newRenderTargetTexture = {};

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_newRenderTargetTexture.Create(a_device,
																	   a_gpuMemoryAllocator,
																	   a_clientSize,
																	   a_rtvDescriptorPool,
																	   a_srvDescriptorPool),
																	   "リサイズ後のRenderTargetTextureの作成に失敗しており、リサイズ処理に失敗しました。",
																	   false);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext), "古いRenderTargetTextureの遅延解放登録に失敗しており、リサイズ処理に失敗しました。", false);

	*this = std::move(l_newRenderTargetTexture);

	return true;
}

bool FWK::Graphics::RenderTargetTexture::CreateGPUResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const Struct::ClientSize& a_clientSize)
{
	D3D12_CLEAR_VALUE l_clearValue = {};

	l_clearValue.Format = m_format;

	l_clearValue.Color[k_clearColorIndexR] = Constant::k_defaultBackBufferClearColor.R();
	l_clearValue.Color[k_clearColorIndexG] = Constant::k_defaultBackBufferClearColor.G();
	l_clearValue.Color[k_clearColorIndexB] = Constant::k_defaultBackBufferClearColor.B();
	l_clearValue.Color[k_clearColorIndexA] = Constant::k_defaultBackBufferClearColor.A();

	const auto l_resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_format,
															 a_clientSize.m_width,
															 a_clientSize.m_height,
															 Constant::k_defaultTexture2DArraySize,
															 Constant::k_defaultTexture2DMipLevels,
															 Constant::k_defaultSampleQuality,
															 Constant::k_defaultSampleQuality,
															 D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_gpuMemoryAllocator.CreateTextureResource(l_resourceDesc, 
																				 &l_clearValue, 
																				 D3D12_RESOURCE_STATE_RENDER_TARGET,
																				 m_gpuResource),
																				 "RenderTargetTexture用TextureResourceの作成に失敗しました。",
																				 false);

	m_width  = a_clientSize.m_width;
	m_height = a_clientSize.m_height;

	return true;
}
bool FWK::Graphics::RenderTargetTexture::CreateRTV(const Device& a_device, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool)
{
	return false;
}
bool FWK::Graphics::RenderTargetTexture::CreateSRV(const Device& a_device, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	return false;
}

bool FWK::Graphics::RenderTargetTexture::ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	return false;
}

bool FWK::Graphics::RenderTargetTexture::IsValidClientSize(const Struct::ClientSize& a_clientSize) const
{
	return false;
}
bool FWK::Graphics::RenderTargetTexture::IsSameSize(const Struct::ClientSize& a_clientSize) const
{
	return false;
}