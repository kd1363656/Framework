#pragma once

namespace FWK::Graphics
{
	template<D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
	class DescriptorPool
	{
	public:

		 DescriptorPool() = default;
		~DescriptorPool() = default;

		void Deserialize(const nlohmann::json& a_rootJson)
		{
			if (a_rootJson.is_null()) { return; }

			m_jsonConverter.Deserialize(a_rootJson, *this);
		}
		bool Create(const Device& a_device)
		{
			// DescriptorHeapの作成数はDescriptorHeapIndexAllocatorの管理数に依存させる
			const auto l_descriptorNUM = m_descriptorIndexAllocator.GetVALIndexCapacity();

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_descriptorNUM == Constant::k_invalidDescriptorIndex, "DescriptorHeapIndexAllocatorの管理数が無効になっており、DescriptorPoolの作成に失敗しました。", false);
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_descriptorNUM == Constant::k_emptyDescriptorNUM,	 "DescriptorHeapIndexAllocatorの管理数が0のため、DescriptorPoolの作成に失敗しました。",			 false);

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_cpuDescriptorHeap.Create(a_device,
																		  HeapType, 
																		  D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 
																		  l_descriptorNUM), 
																		  "CPUDescriptorHeapの生成に失敗しました。",
																		  false);

			// CBV_SRV_UAV/SAMPLERの場合だけ、ShaderVisibleDescriptorHeapを自動作成する。
			// RTV/DSVの場合は、このif constexprの中身がコンパイル時に無効化される
			if constexpr (IsShaderVisibleSupportedDescriptorHeapType())
			{
				m_shaderVisibleDescriptorHeap = std::make_shared<DescriptorHeap>();

				FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_shaderVisibleDescriptorHeap->Create(a_device,
																				         HeapType,
																						 D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
																						 l_descriptorNUM),
																						 "ShaderVisibleDescriptorHeap",
																						 false);
			}

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_descriptorIndexAllocator.Create(), "DescriptorHeapIndexAllocatorの作成に失敗しました。", false);

			return true;
		}

		nlohmann::json Serialize() const
		{
			return m_jsonConverter.Serialize(*this);
		}

		TypeAlias::DescriptorIndex Allocate()
		{
			return m_descriptorIndexAllocator.Allocate();
		}

		void Release(const TypeAlias::DescriptorIndex a_index)
		{
			FWK_ASSERT_RETURN_IF_FAILED(a_index == Constant::k_invalidDescriptorIndex, "無効なDescriptorIndexを解放しようとしており、DescriptorIndexの解放に失敗しました。");

			m_descriptorIndexAllocator.Release(a_index);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE FetchVALCPUDescriptorHandle(const TypeAlias::DescriptorIndex a_index) const
		{
			return m_cpuDescriptorHeap.FetchVALCPUDescriptorHandle(a_index);
		}
		D3D12_CPU_DESCRIPTOR_HANDLE FetchVALShaderVisibleCPUDescriptorHandle(const TypeAlias::DescriptorIndex a_index) const
		{
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_shaderVisibleDescriptorHeap, "ShaderVisibleDescriptorHeapが作成されておらず、ShaderVisibleDescriptorHandleの取得に失敗しました。", {});

			return m_shaderVisibleDescriptorHeap->FetchVALCPUDescriptorHandle(a_index);
		}

		D3D12_GPU_DESCRIPTOR_HANDLE FetchVALGPUDescriptorHandle(const TypeAlias::DescriptorIndex a_index) const
		{
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_shaderVisibleDescriptorHeap, "ShaderVisibleDescriptorHeapが作成されておらず、GPUDescriptorHandleの取得に失敗しました。", {});

			return m_shaderVisibleDescriptorHeap->FetchVALGPUDescriptorHandle(a_index);
		}

		void CopyCPUDescriptorToShaderVisibleDescriptor(const Device& a_device, const TypeAlias::DescriptorIndex a_index) const
		{
			FWK_ASSERT_RETURN_IF_FAILED(!m_shaderVisibleDescriptorHeap,				   "ShaderVisibleDescriptorHeapが作成されておらず、Descriptorのコピーに失敗しました");
			FWK_ASSERT_RETURN_IF_FAILED(a_index == Constant::k_invalidDescriptorIndex, "無効なDescriptorIndexが指定されており、Descriptorのコピーに失敗しました。");

			const auto& l_device = a_device.GetREFDevice();

			FWK_ASSERT_RETURN_IF_FAILED(!l_device, "Deviceが作成されておらず、Descriptorのコピーに失敗しました。");

			const auto l_sourceCPUDescriptorHandle      = m_cpuDescriptorHeap.FetchVALCPUDescriptorHandle			(a_index);
			const auto l_destinationCPUDescriptorHandle = m_shaderVisibleDescriptorHeap->FetchVALCPUDescriptorHandle(a_index);

			// CPUOnly側に作成したディスクリプタをShaderVisible側へコピーする
			// CopyDescriptorsSimple(コピーするディスクリプタ数、
			//						 コピー先のCPUディスクリプタハンドル、
			//						 コピー元のCPUディスクリプタハンドル、
			//						 コピーするディスクリプタヒープの種類);
			l_device->CopyDescriptorsSimple(k_copyOnceDescriptorCount,
											l_destinationCPUDescriptorHandle,
											l_sourceCPUDescriptorHandle,
											HeapType);
		}

		const auto& GetREFShaderVisibleDescriptorHeap() const { return m_shaderVisibleDescriptorHeap; }

		const auto& GetREFCPUDescriptorHeap       () const { return m_cpuDescriptorHeap; }
		const auto& GetREFDescriptorIndexAllocator() const { return m_descriptorIndexAllocator; }

		auto& GetMutableREFDescriptorIndexAllocator() { return m_descriptorIndexAllocator; }

	private:

		static constexpr bool IsShaderVisibleSupportedDescriptorHeapType()
		{
			if		constexpr (HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) { return true; }
			else if constexpr (HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)     { return true; }
			else																   { return false; }
		}

		static constexpr UINT k_copyOnceDescriptorCount = 1U;

		std::shared_ptr<DescriptorHeap> m_shaderVisibleDescriptorHeap = nullptr;

		DescriptorHeap				 m_cpuDescriptorHeap	    = {};
		DescriptorHeapIndexAllocator m_descriptorIndexAllocator = {};

		Converter::DescriptorPoolJsonConverter<HeapType> m_jsonConverter = {};
	};
}