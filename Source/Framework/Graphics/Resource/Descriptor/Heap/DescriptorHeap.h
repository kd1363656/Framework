#pragma once

namespace FWK::Graphics
{
	class DescriptorHeap final
	{
	public:

		 DescriptorHeap() = default;
		~DescriptorHeap() = default;

		bool Create(const Device&					  a_device, 
					const D3D12_DESCRIPTOR_HEAP_TYPE  a_type,
					const D3D12_DESCRIPTOR_HEAP_FLAGS a_flags,
					const UINT						  a_num);

		bool IsShaderVisible       ()									      const;
		bool IsValidDescriptorIndex(const TypeAlias::DescriptorIndex a_index) const;

		D3D12_CPU_DESCRIPTOR_HANDLE FetchVALCPUDescriptorHandle(const TypeAlias::DescriptorIndex a_index) const;
		D3D12_GPU_DESCRIPTOR_HANDLE FetchVALGPUDescriptorHandle(const TypeAlias::DescriptorIndex a_index) const;

		const auto& GetREFDescriptorHeap() const { return m_descriptorHeap; }

		UINT GetVALDescriptorNUM () const { return m_num; }
		UINT GetVALDescriptorSize() const { return m_size; }

	private:

		bool CanUseShaderVisibleFlag(const D3D12_DESCRIPTOR_HEAP_TYPE a_type) const;

		static constexpr UINT k_invalidSize = 0U;

		TypeAlias::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap = nullptr;

		D3D12_DESCRIPTOR_HEAP_TYPE  m_descriptorHeapType  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		D3D12_DESCRIPTOR_HEAP_FLAGS m_descriptorHeapFlags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		UINT m_num  = Constant::k_invalidDescriptorNUM;
		UINT m_size = k_invalidSize;
	};
}