#include "GPUMemoryAllocator.h"

bool FWK::Graphics::GPUMemoryAllocator::Create(const Device& a_device)
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_device, "デバイスが作成されておらず、GPUメモリアロケータの作成に失敗しました。", false);

    const auto& l_adapter = a_device.GetREFAdapter();

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_adapter, "GPUアダプターが保存されておらず、GPUメモリアロケータの作成に失敗しました。", false);

    D3D12MA::ALLOCATOR_DESC l_allocatorDesc = {};

    // D3D12MA::ALLOCATOR_DESCについて
    // pDevice  : D3D12MAがリソース作成に使用するD3D12デバイス
    // pAdapter : D3D12MAがメモリ情報を取得に使用するGPUアダプター
    l_allocatorDesc.pDevice  = l_device.Get ();
    l_allocatorDesc.pAdapter = l_adapter.Get();

    // D3D12MA::CreateAllocator(アロケータ作成設定、
    //                          作成されたD3D12MAアロケータの受け取り先);
    const auto l_hr = D3D12MA::CreateAllocator(&l_allocatorDesc, m_allocator.ReleaseAndGetAddressOf());

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "D3D12MAアロケータの作成に失敗しており、、GPUメモリアロケータの作成に失敗しました。", false);

    return true;
}

bool FWK::Graphics::GPUMemoryAllocator::CreateTextureResource(const D3D12_RESOURCE_DESC&  a_resourceDesc, 
                                                              const D3D12_CLEAR_VALUE*    a_clearValue, 
                                                              const D3D12_RESOURCE_STATES a_initialResourceState, 
                                                                    Struct::GPUResource&  a_gpuResource) const
{
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_allocator, "D3D12MAアロケータが作成されておらず、TextureResourceの作成に失敗しました。", false);

    D3D12MA::ALLOCATION_DESC l_allocationDesc = {};

    // D3D12MA::ALLOCATION_DESCについて
    // HeapType : リソースをどの種類のGPUメモリに置くか
    // TextureResourceはGPU側で使用する本番リソースなのでDEFAULTを指定する
    l_allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    // D3D12MA::Allocator::CreateResource(割り当て設定、
    //                                    作成するリソース設定、
    //                                    作成直後のリソースステート、
    //                                    ClearValue(nullptrなので未使用)、
    //                                    D3D12MA側Allocationの受取先、
    //                                    受け取りたいResourceインターフェース型ID、
    //                                    作成されたResourceの受取先);
    const auto l_hr = m_allocator->CreateResource(&l_allocationDesc,
                                                  &a_resourceDesc,
                                                  a_initialResourceState,
                                                  a_clearValue,
                                                  a_gpuResource.m_allocation.ReleaseAndGetAddressOf(),
                                                  IID_PPV_ARGS(a_gpuResource.m_resource.ReleaseAndGetAddressOf()));

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "D3D12MAによるTextureResourceの作成に失敗しました。", false);

    return true;
}

bool FWK::Graphics::GPUMemoryAllocator::CreateBufferResource(const UINT64& a_bufferSize, const D3D12_RESOURCE_STATES a_initialResourceState, Struct::GPUResource& a_gpuResource) const
{
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_allocator,                                  "D3D12MAアロケータが作成されておらず、BufferResourceの作成に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_bufferSize == Constant::k_invalidBufferSize, "BufferResourceの作成サイズが0のため、BufferResourceの作成に失敗しました。", false);

    D3D12MA::ALLOCATION_DESC l_allocationDesc = {};

    // D3D12MA::ALLOCATION_DESCについて
    // HeapType : リソースをどの種類のGPUメモリに置くか
    l_allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    // D3D12_RESOURCE_DESCについて
    // Buffer(作成するバッファサイズ)
    // VertexBuffer / IndexBuffer / MeshletBufferなどの線形バッファとして扱う
    const auto l_resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(a_bufferSize);

    // D3D12MA::Allocator::CreateResource(割り当て設定、
    //                                    作成するリソース設定、
    //                                    作成直後のリソースステート、
    //                                    ClearValue(nullptrなので未使用)、
    //                                    D3D12MA側Allocationの受取先、
    //                                    受け取りたいResourceインターフェース型ID、
    //                                    作成されたResourceの受取先);
    const auto l_hr = m_allocator->CreateResource(&l_allocationDesc,
                                                  &l_resourceDesc,
                                                  a_initialResourceState,
                                                  nullptr,
                                                  a_gpuResource.m_allocation.ReleaseAndGetAddressOf(),
                                                  IID_PPV_ARGS(a_gpuResource.m_resource.ReleaseAndGetAddressOf()));

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "D3D12MAによるBufferResourceの作成に失敗しました。", false);

    return true;
}