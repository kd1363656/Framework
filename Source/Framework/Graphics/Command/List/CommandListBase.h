#pragma once

namespace FWK::Graphics
{
	class CommandListBase
	{
	public:

		explicit CommandListBase(const D3D12_COMMAND_LIST_TYPE a_createCommandListType);
		virtual ~CommandListBase();

		bool Create(const Device& a_device);

		virtual void Reset(const CommandAllocatorBase& a_commandAllocator);

		void Close() const;

		const auto& GetREFCommandList() const { return m_commandList; }

		auto GetVALCreateCommandListType() const { return k_createCommandListType; }

	protected:

		void ExecuteResourceBarrier(const D3D12_RESOURCE_BARRIER& a_resourceBarrier) const;

		template <D3D12_DESCRIPTOR_HEAP_TYPE Type>
		void SetupShaderVisibleDescriptorHeap(const DescriptorPool<Type>& a_descriptorPool) const
		{
			const auto& l_directCommandList = GetREFCommandList();

			FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、ディスクリプタヒープの設定ができませんでした。");

			const auto& l_shaderVisibleDescriptorHeap = a_descriptorPool.GetREFShaderVisibleDescriptorHeap();

			FWK_ASSERT_RETURN_IF(!l_shaderVisibleDescriptorHeap, "ShaderVisibleなディストラクタヒープが無効になっており、ディスクリプタヒープの設定ができませんでした。");

			const auto& l_descriptorHeap = l_shaderVisibleDescriptorHeap->GetREFDescriptorHeap();

			FWK_ASSERT_RETURN_IF(!l_descriptorHeap, "ShaderVisibleなディスクリプタヒープが作成されておらず、ディスクリプタヒープの設定ができませんでした。");

			ID3D12DescriptorHeap* l_descriptorHeapList[] =
			{
				l_descriptorHeap.Get()
			};

			// シェーダーから参照するDescriptorHeapを設定する
			// SetDescriptorHeap(設定するヒープ数,
			//					 ヒープ配列の先頭アドレス);
			l_directCommandList->SetDescriptorHeaps(k_setDescriptorHeapNUM, l_descriptorHeapList);
		}

		template <typename Type>
		bool IsSameWeakOwner(const std::weak_ptr<Type>& a_left, const std::weak_ptr<Type>& a_right) const
		{
			return !a_left.owner_before (a_right) &&
				   !a_right.owner_before(a_left);
		}

	private:

		static constexpr UINT k_singleSetupBarrierNUM = 1U;
		static constexpr UINT k_setDescriptorHeapNUM  = 1U;

		const D3D12_COMMAND_LIST_TYPE k_createCommandListType;

		TypeAlias::ComPtr<ID3D12GraphicsCommandList6> m_commandList;
	};
}