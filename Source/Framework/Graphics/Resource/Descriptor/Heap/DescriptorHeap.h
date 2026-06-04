#pragma once

namespace FWK::Graphics
{
	class DescriptorHeap final
	{
	public:

		 DescriptorHeap() = default;
		~DescriptorHeap() = default;

		bool Create(const Device&					  a_device, 
					const D3D12_DESCRIPTOR_HEAP_TYPE  a_descriptorHeapType,
					const D3D12_DESCRIPTOR_HEAP_FLAGS a_descriptorHeapFlags,
					const UINT						  a_descriptorNUM);

		bool IsShaderVisible       ()													const;
		bool IsValidDescriptorIndex(const TypeAlias::DescriptorIndex a_descriptorIndex) const;

		D3D12_CPU_DESCRIPTOR_HANDLE FetchVALCPUDescriptorHandle(const TypeAlias::DescriptorIndex a_descriptorIndex) const;
		D3D12_GPU_DESCRIPTOR_HANDLE FetchVALGPUDescriptorHandle(const TypeAlias::DescriptorIndex a_descriptorIndex) const;

		UINT GetVALDescriptorNUM () const { return m_descriptorNUM; }
		UINT GetVALDescriptorSize() const { return m_descriptorSize; }

	private:

		bool CanUseShaderVisibleFlag(const D3D12_DESCRIPTOR_HEAP_TYPE a_descriptorHeapType) const;

		static constexpr UINT k_emptyDescriptorSize = 0U;

		TypeAlias::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap = nullptr;

		D3D12_DESCRIPTOR_HEAP_TYPE  m_descriptorHeapType  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		D3D12_DESCRIPTOR_HEAP_FLAGS m_descriptorHeapFlags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		UINT m_descriptorNUM  = Constant::k_emptyDescriptorNUM;
		UINT m_descriptorSize = k_emptyDescriptorSize;
	};
}