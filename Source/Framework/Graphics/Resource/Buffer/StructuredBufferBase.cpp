#include "StructuredBufferBase.h"

FWK::Graphics::StructuredBufferBase::StructuredBufferBase(StructuredBufferBase&& a_other) noexcept
{
	MoveFrom(std::move(a_other));
}

FWK::Graphics::StructuredBufferBase& FWK::Graphics::StructuredBufferBase::operator=(StructuredBufferBase&& a_other) noexcept
{
	if (this == &a_other) { return *this; }

	MoveFrom(std::move(a_other));

	return *this;
}

void FWK::Graphics::StructuredBufferBase::ReleaseImmediatelySRVDescriptorIndex(TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	if (m_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex) { return; }

	a_cbvSRVUAVDescriptorPool.Release(m_srvDescriptorIndex);

	m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::StructuredBufferBase::CreateSRV(const Device&                             a_device,
																			   const Struct::GPUResource&                a_bufferGPUResource,
	                                                                           const UINT                                a_elementCount, 
	                                                                           const UINT                                a_structureByteStride,
	                                                                                 TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool) const
{
	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF(!l_device,                                              "デバイスが作成されておらず、StructuredBuffer用SRVの作成に失敗しました。",   DescriptorHeap::k_invalidDescriptorIndex);
	FWK_ASSERT_RETURN_VALUE_IF(!a_bufferGPUResource.m_resource,                        "BufferResourceが無効のため、StructuredBuffer用SRVの作成に失敗しました。",   DescriptorHeap::k_invalidDescriptorIndex);
	FWK_ASSERT_RETURN_VALUE_IF(a_elementCount == k_invalidElementCount,                "ElementCountが0のため、StructuredBuffer用SRVの作成に失敗しました。",        DescriptorHeap::k_invalidDescriptorIndex);
	FWK_ASSERT_RETURN_VALUE_IF(a_structureByteStride == k_invalidStructuredByteStride, "StructureByteStrideが0のため、StructuredBuffer用SRVの作成に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

	const auto l_srvDescriptorIndex = a_cbvSRVUAVDescriptorPool.Allocate();

	D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};

	// D3D12_SHADER_RESOURCE_VIEW_DESCについて
	// Shader4ComponentMapping : Shader側でRGBA部分をどのように読むか
	// Format                  : StructuredBufferなのでDXGI_FORMAT_UNKNOWNを指定する
	// ViewDimension		   : BufferをSRVとして参照するためBUFFERを指定する
	l_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	l_srvDesc.Format				  = DXGI_FORMAT_UNKNOWN;
	l_srvDesc.ViewDimension			  = D3D12_SRV_DIMENSION_BUFFER;

	// D3D12_BUFFER_SRVについて
	// FirstElement         : 先頭要素番号
	// NumElements          : StructuredBufferとして参照する要素数
	// StructureByteStride  : 1要素あたりのバイトサイズ
	// Flags				: RawBufferではないためNONEを指定する
	l_srvDesc.Buffer.FirstElement        = k_firstStructuredBufferElement;
	l_srvDesc.Buffer.NumElements         = a_elementCount;
	l_srvDesc.Buffer.StructureByteStride = a_structureByteStride;
	l_srvDesc.Buffer.Flags				 = D3D12_BUFFER_SRV_FLAG_NONE;

	const auto l_cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(a_cbvSRVUAVDescriptorPool.FetchVALCPUDescriptorHandle(l_srvDescriptorIndex));

	// CreateShaderResourceView(BufferResource, 
	//							SRV設定、
	//							CPUOnlyDescriptorHeap側のCPUHandle);
	l_device->CreateShaderResourceView(a_bufferGPUResource.m_resource.Get(), &l_srvDesc, l_cpuHandle);

	if (!a_cbvSRVUAVDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(a_device, l_srvDescriptorIndex))
	{
		a_cbvSRVUAVDescriptorPool.Release(l_srvDescriptorIndex);

		FWK_ASSERT_RETURN_VALUE("CPUOnlyからShaderVisibleSRVへのコピーに失敗したため、StructuredBuffer用SRVの作成に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);
	}

	return l_srvDescriptorIndex;
}

bool FWK::Graphics::StructuredBufferBase::ReserveReleaseCurrentResource(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	// 既に解放するものがなければreturn
	if (!m_bufferGPUResource.m_resource && 
		m_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex)
	{	
		return true;
	}

	FWK_ASSERT_RETURN_VALUE_IF(!m_bufferGPUResource.m_resource,                                  "StructuredBufferのGPUResourceが無効のため、遅延解放登録に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(m_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "StructuredBufferのSRVDescriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue  == Fence::k_unusedFenceValue,                "FenceValueが無効のため、StructuredBufferの遅延解放登録に失敗しました。",         false);

	// GPUリソース、ディスクリプタインデックスの適切なタイミングでの解放を予約
	ResourceReleaseContext::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

	l_gpuResourceReleaseRecord.m_gpuResource       = std::move(m_bufferGPUResource);
	l_gpuResourceReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	ResourceReleaseContext::DescriptorIndexReleaseRecord l_srvDescriptorIndexReleaseRecord = {};

	l_srvDescriptorIndexReleaseRecord.m_descriptorIndex   = m_srvDescriptorIndex;
	l_srvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord(std::move(l_gpuResourceReleaseRecord)),               "StructuredBufferのGPUResourceを遅延解放Queueへ登録できませんでした。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseCBVSRVUAVDescriptorIndex(std::move(l_srvDescriptorIndexReleaseRecord)), "StructuredBufferのSRVDescriptorIndexを遅延解放Queueへ登録できませんでした。", false);

	// もう一度処理が走らないように初期化
	m_bufferGPUResource = {};
	m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

	return true;
}

void FWK::Graphics::StructuredBufferBase::MoveFrom(StructuredBufferBase&& a_other) noexcept
{
	m_bufferGPUResource  = std::move(a_other.m_bufferGPUResource);
	m_srvDescriptorIndex = a_other.m_srvDescriptorIndex;

	a_other.m_bufferGPUResource  = {};
	a_other.m_srvDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
}