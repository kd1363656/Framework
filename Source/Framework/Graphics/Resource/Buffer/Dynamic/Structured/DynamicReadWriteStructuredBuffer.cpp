#include "DynamicReadWriteStructuredBuffer.h"

FWK::Graphics::DynamicReadWriteStructuredBuffer::DynamicReadWriteStructuredBuffer() : 
    m_uavDescriptorIndex(DescriptorHeap::k_invalidDescriptorIndex),

    m_currentResourceState(D3D12_RESOURCE_STATE_COMMON),

    m_elementCount(k_invalidElementCount),

    m_structureByteStride(k_invalidStructuredByteStride)
{}
FWK::Graphics::DynamicReadWriteStructuredBuffer::~DynamicReadWriteStructuredBuffer()
{
    Release();
}

FWK::Graphics::DynamicReadWriteStructuredBuffer::DynamicReadWriteStructuredBuffer(DynamicReadWriteStructuredBuffer && a_other) noexcept : 
    StructuredBufferBase(std::move(a_other)),

    m_uavDescriptorIndex(DescriptorHeap::k_invalidDescriptorIndex),

    m_currentResourceState(D3D12_RESOURCE_STATE_COMMON),

    m_elementCount(k_invalidElementCount),

    m_structureByteStride(k_invalidStructuredByteStride)
{
    MoveFrom(std::move(a_other));
}

FWK::Graphics::DynamicReadWriteStructuredBuffer& FWK::Graphics::DynamicReadWriteStructuredBuffer::operator=(DynamicReadWriteStructuredBuffer&& a_other) noexcept
{
    if (this == &a_other) { return *this; }

    // 自身が現在持っているResourceを先に解放予約する
    Release();

    StructuredBufferBase::operator=(std::move(a_other));

    MoveFrom(std::move(a_other));

    return *this;
}

bool FWK::Graphics::DynamicReadWriteStructuredBuffer::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
    // 既に解放するものがなければreturn
    if (!GetREFBufferGPUResource().m_resource && 
        GetVALSRVDescriptorIndex() == DescriptorHeap::k_invalidDescriptorIndex &&
        m_uavDescriptorIndex       == DescriptorHeap::k_invalidDescriptorIndex)
    {
        return true;
    }

    FWK_ASSERT_RETURN_VALUE_IF(!GetREFBufferGPUResource().m_resource,                                  "DynamicReadWriteStructuredBufferのGPUResourceが無効のため、遅延解放登録に失敗しました。",        false);
    FWK_ASSERT_RETURN_VALUE_IF(GetVALSRVDescriptorIndex() == DescriptorHeap::k_invalidDescriptorIndex, "DynamicReadWriteStructuredBufferのSRVDescriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(m_uavDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex,       "DynamicReadWriteStructuredBufferのUAVDescriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue == Constant::k_unusedFenceValue,                    "FenceValueが無効のため、DynamicReadWriteStructuredBufferの遅延解放登録に失敗しました。",         false);

    // UAVDescirptorIndexの遅延解放Recordを作る
    ResourceReleaseContext::DescriptorIndexReleaseRecord l_uavDescriptorIndexReleaseRecord = {};

    l_uavDescriptorIndexReleaseRecord.m_descriptorIndex   = m_uavDescriptorIndex;
    l_uavDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

    FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseCBVSRVUAVDescriptorIndex(std::move(l_uavDescriptorIndexReleaseRecord)), "DynamicReadWriteStructuredBufferのUAVDescriptorIndexを遅延解放Queueへ登録できませんでした。", false);

    // UAVDescriptorIndexは遅延解放Queueへ登録したので、このクラスでは無効値に戻す
    m_uavDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

    FWK_ASSERT_RETURN_VALUE_IF(!ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext), "DynamicReadWriteStructuredBufferのGPUResourceとSRVDescriptorIndexを遅延解放Queueへ登録できませんでした。", false);

    m_currentResourceState = D3D12_RESOURCE_STATE_COMMON;
    m_elementCount         = k_invalidElementCount;
    m_structureByteStride  = k_invalidStructuredByteStride;

    return true;
}

void FWK::Graphics::DynamicReadWriteStructuredBuffer::Release()
{
    // 既に解放するものがなければreturn
    if (!GetREFBufferGPUResource().m_resource && 
        GetVALSRVDescriptorIndex() == DescriptorHeap::k_invalidDescriptorIndex &&
        m_uavDescriptorIndex       == DescriptorHeap::k_invalidDescriptorIndex)
    {
        return;
    }

    auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();

    const auto& l_renderer           = l_graphicsManager.GetREFRenderer   ();
    const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue();

    auto& l_resourceContext        = l_graphicsManager.GetMutableREFResourceContext       ();
    auto& l_resourceReleaseContext = l_resourceContext.GetMutableREFResourceReleaseContext();

    const auto& l_retiredFenceValue = l_directCommandQueue.FetchREFLastSignaledFenceValue();

    // StaticStructuredBufferと同じく、
    // 現在のDirectCommandQueueのFenceValueを基準に遅延解放する
    FWK_ASSERT_RETURN_IF(!ReserveRelease(l_retiredFenceValue, l_resourceReleaseContext), "DynamicReadWriteStructuredBufferの自動遅延解放登録に失敗しました。");
}

void FWK::Graphics::DynamicReadWriteStructuredBuffer::ReleaseImmediatelyUAVDescriptorIndex(TypeAlias::CBVSRVUAVDescriptorPool & a_cbvSRVUAVDescriptorPool)
{
    if (m_uavDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex) { return; }

    a_cbvSRVUAVDescriptorPool.Release(m_uavDescriptorIndex);

    m_uavDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
}

void FWK::Graphics::DynamicReadWriteStructuredBuffer::ReleaseImmediatelyDescriptorIndices(TypeAlias::CBVSRVUAVDescriptorPool & a_cbvSRVUAVDescriptorPool)
{
    // SRVは基底クラスが持っているため、基底側の即時解放関数を呼ぶ
    ReleaseImmediatelySRVDescriptorIndex(a_cbvSRVUAVDescriptorPool);

    // UAVはこのクラスが持っているため、このクラス側で解放する
    ReleaseImmediatelyUAVDescriptorIndex(a_cbvSRVUAVDescriptorPool);
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::DynamicReadWriteStructuredBuffer::CreateUAV(const Device&                              a_device, 
                                                                                           const ResourceReleaseContext::GPUResource& a_bufferGPUResource, 
                                                                                           const UINT                                 a_elementCount, 
                                                                                           const UINT                                 a_structureByteStride, 
                                                                                                 TypeAlias::CBVSRVUAVDescriptorPool&  a_cbvSRVUAVDescriptorPool) const
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device,                                              "デバイスが作成されておらず、DynamicReadWriteStructuredBuffer用UAVの作成に失敗しました。",   DescriptorHeap::k_invalidDescriptorIndex);
	FWK_ASSERT_RETURN_VALUE_IF(!a_bufferGPUResource.m_resource,                        "BufferResourceが無効のため、DynamicReadWriteStructuredBuffer用UAVの作成に失敗しました。",   DescriptorHeap::k_invalidDescriptorIndex);
	FWK_ASSERT_RETURN_VALUE_IF(a_elementCount == k_invalidElementCount,                "ElementCountが0のため、DynamicReadWriteStructuredBuffer用UAVの作成に失敗しました。",        DescriptorHeap::k_invalidDescriptorIndex);
	FWK_ASSERT_RETURN_VALUE_IF(a_structureByteStride == k_invalidStructuredByteStride, "StructureByteStrideが0のため、DynamicReadWriteStructuredBuffer用UAVの作成に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    const auto l_uavDescriptorIndex = a_cbvSRVUAVDescriptorPool.Allocate();

    FWK_ASSERT_RETURN_VALUE_IF(l_uavDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "UAV用DescriptorIndexの確保に失敗したため、DynamicReadWriteStructuredBuffer用UAVの作成に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    D3D12_UNORDERED_ACCESS_VIEW_DESC l_uavDesc = {};

    // D3D12_UNORDERED_ACCESS_VIEW_DESCについて
    // Format        : StructuredBufferは要素の型をHLSL側の構造体で解釈するため、DXGI_FORMAT_UNKNOWNを指定する
    // ViewDimension : 今回はTextureではなくBufferをUAVとして扱うため、D3D12_UVA_DIMENSION_BUFFERを指定する
    l_uavDesc.Format        = DXGI_FORMAT_UNKNOWN;
    l_uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

    // D3D12_BUFFER_UAVについて
    // FirstElement         : 何番目の要素からUAVとして見せるか
    // NumElements          : StructuredBufferとして扱う要素数
    // StructureByteStride  : 1要素辺りのバイトサイズ
    // CounterOffsetInBytes : AppendStructuredBufferやConsumeStructuredBuffer用のCounter位置、RWStructuredBufferなら使用しない
    // Flags                : RawBufferではないためNone
    l_uavDesc.Buffer.FirstElement         = k_firstStructuredBufferElement;
    l_uavDesc.Buffer.NumElements          = a_elementCount;
    l_uavDesc.Buffer.StructureByteStride  = a_structureByteStride;
    l_uavDesc.Buffer.CounterOffsetInBytes = k_counterOffsetInBytes;
    l_uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

    const auto l_cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(a_cbvSRVUAVDescriptorPool.FetchVALCPUDescriptorHandle(l_uavDescriptorIndex));

    // CreateUnorderedAccessView(UAVとして見せたいGPUResource,
    //                           CounterResource(Append/Consume系統でなければnullptr),
    //                           UAVの設定,
    //                           CPUOnlyDescriptorHandleのCPUHandle);
    l_device->CreateUnorderedAccessView(a_bufferGPUResource.m_resource.Get(),
                                        nullptr,
                                        &l_uavDesc,
                                        l_cpuHandle);

    if (!a_cbvSRVUAVDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(a_device, l_uavDescriptorIndex))
    {
        a_cbvSRVUAVDescriptorPool.Release(l_uavDescriptorIndex);

        FWK_ASSERT_RETURN_VALUE("CPUOnlyからShaderVisibleUAVへのコピーに失敗したため、DynamicReadWriteStructuredBuffer用UAVの作成に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);
    }

    return l_uavDescriptorIndex;
}

void FWK::Graphics::DynamicReadWriteStructuredBuffer::MoveFrom(DynamicReadWriteStructuredBuffer&& a_other) noexcept
{
    // 基底側のGPUResourceとSRVDescriptorIndexは、StructuredBuffer側のMove処理で移動済み
    // このクラス固有のメンバだけを移動
    m_uavDescriptorIndex   = a_other.m_uavDescriptorIndex;
    m_currentResourceState = a_other.m_currentResourceState;
    m_elementCount         = a_other.m_elementCount;
    m_structureByteStride  = a_other.m_structureByteStride;

    // デストラクタで二重開放しないようにする
    a_other.m_uavDescriptorIndex   = DescriptorHeap::k_invalidDescriptorIndex;
    a_other.m_currentResourceState = D3D12_RESOURCE_STATE_COMMON;
    a_other.m_elementCount         = k_invalidElementCount;
    a_other.m_structureByteStride  = k_invalidStructuredByteStride;
}