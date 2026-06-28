#include "DescriptorHeap.h"

bool FWK::Graphics::DescriptorHeap::Create(const Device&					     a_device, 
										   const D3D12_DESCRIPTOR_HEAP_TYPE  a_type, 
										   const D3D12_DESCRIPTOR_HEAP_FLAGS a_flags, 
										   const UINT						 a_num)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_num == Constant::k_invalidDescriptorNUM, "DescriptorHeapのDescriptor数が無効のため、DescriptorHeapの作成に失敗しました。", false);

	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF(!l_device, "Deviceが作成されておらず、DescriptorHeapの作成に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(a_flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE &&
							   !CanUseShaderVisibleFlag(a_type),
							   "指定されたDescriptorHeapTypeではShaderVisibleを使用できないため、DescriptorHeapの作成に失敗しました。",
							   false);

	m_descriptorHeapType  = a_type;
	m_descriptorHeapFlags = a_flags;
	m_num				  = a_num;

	// ディスクリプタヒープ作成設定を入れる構造体
	D3D12_DESCRIPTOR_HEAP_DESC l_desc = {};

	// このヒープの種類を設定する(RTV / DSV / CBV_SRV_UAV / SAMPLERの内どれか)
	l_desc.Type = a_type;

	// このヒープに何個ディスクリプタを入れるか
	l_desc.NumDescriptors = m_num;

	// ヒープをシェーダーから見えるようにするかどうか
	l_desc.Flags = m_descriptorHeapFlags;

	// どのGPUノードで使用するかを指定する
	l_desc.NodeMask = Constant::k_defaultGPUNodeMask;

	// ディスクリプタヒープを作成する関数
	// CreateDescriptorHeap(設定、
	//					    受け取りたいCOMインターフェース型のID、
	//					    作成結果のポインタを書き込むアドレス);
	auto l_hr = l_device->CreateDescriptorHeap(&l_desc, IID_PPV_ARGS(m_descriptorHeap.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "ディスクリプタヒープの作成に失敗しました。", false);

	// DescriptorHandleをindexから計算するために、
	// Descriptor一個分のHandleサイズを取得する
	m_size = l_device->GetDescriptorHandleIncrementSize(m_descriptorHeapType);

	FWK_ASSERT_RETURN_VALUE_IF(m_size == k_invalidSize, "DescriptorHandleの増分サイズ取得に失敗しました。", false);

	return true;
}

bool FWK::Graphics::DescriptorHeap::IsShaderVisible() const
{
	return m_descriptorHeapFlags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
}
bool FWK::Graphics::DescriptorHeap::IsValidDescriptorIndex(const TypeAlias::DescriptorIndex a_index) const
{
	if (a_index == Constant::k_invalidDescriptorIndex ||
		a_index >= m_num)
	{
		return false; 
	}
	
	return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE FWK::Graphics::DescriptorHeap::FetchVALCPUDescriptorHandle(const TypeAlias::DescriptorIndex a_index) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_descriptorHeap,			     "DescriptorHeapが作成されておらず、CPUDescriptorHandleの取得に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!IsValidDescriptorIndex(a_index), "DescriptorIndexが範囲外のため、CPUDescriptorHandleの取得に失敗しました。",    {});

	// ディスクリプタヒープの先頭CPUHandleからa_descriptorIndex個分だけ進めたCPUHandleを作ってくれる
	return CD3DX12_CPU_DESCRIPTOR_HANDLE { m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(a_index), m_size };
}
D3D12_GPU_DESCRIPTOR_HANDLE FWK::Graphics::DescriptorHeap::FetchVALGPUDescriptorHandle(const TypeAlias::DescriptorIndex a_index) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_descriptorHeap,				 "DescriptorHeapが作成されておらず、GPUDescriptorHandleの取得に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!IsValidDescriptorIndex(a_index), "DescriptorIndexが範囲外のため、GPUDescriptorHandleの取得に失敗しました。",    {});

	// RTV/DSVのようなShaderVisibleではないDescriptorHeapは、GPUDescriptorHandleを使えないためチェックをしておく
	FWK_ASSERT_RETURN_VALUE_IF(!IsShaderVisible(), "ShaderVisibleでないディスクリプタヒープがGPUDescriptorHandleを取得しており、GPUDescriptorHandleの取得に失敗しました。", {});

	// ディスクリプタヒープの先頭GPUHandleからa_descriptorIndex個分だけ進めたCPUHandleを作ってくれる
	return CD3DX12_GPU_DESCRIPTOR_HANDLE { m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(a_index), m_size };
}

bool FWK::Graphics::DescriptorHeap::CanUseShaderVisibleFlag(const D3D12_DESCRIPTOR_HEAP_TYPE a_type) const
{
	switch (a_type)
	{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		{
			return true;
		}
		break;

		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		{
			return false;
		}
		break;

		default:
		{
			return false;
		}
		break;
	}
}