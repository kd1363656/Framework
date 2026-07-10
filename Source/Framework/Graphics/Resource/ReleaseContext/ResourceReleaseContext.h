#pragma once

namespace FWK::Graphics
{
	class ResourceReleaseContext
	{
	public:

		struct DeferredResourceReleaseRecordBase
		{
			UINT64 m_retiredFenceValue = Constant::k_unusedFenceValue;
		};

		struct DescriptorIndexReleaseRecord final : public DeferredResourceReleaseRecordBase
		{
			TypeAlias::DescriptorIndex m_descriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		};

		struct GPUResourceReleaseRecord final : public DeferredResourceReleaseRecordBase
		{
			Struct::GPUResource m_gpuResource = {};
		};

	public:

		bool ReserveDeferredReleaseGPUResourceRecord(GPUResourceReleaseRecord&& a_releaseRecord);

		bool ReserveDeferredReleaseRTVDescriptorIndex      (DescriptorIndexReleaseRecord&& a_releaseRecord);
		bool ReserveDeferredReleaseCBVSRVUAVDescriptorIndex(DescriptorIndexReleaseRecord&& a_releaseRecord);
		bool ReserveDeferredReleaseDSVDescriptorIndex      (DescriptorIndexReleaseRecord&& a_releaseRecord);

		void ReleaseAvailableDeferredResources(const TypeAlias::DirectCommandQueue&      a_directCommandQueue, 
													 TypeAlias::RTVDescriptorPool&       a_rtvDescriptorPool,
													 TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
													 TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool);

		// ※ 注意 この関数を呼び出す前に必ずGPUとの同期をとること
		TypeAlias::DescriptorIndex ReleaseRenderTargetResourceImmediately(const TypeAlias::DescriptorIndex a_rtvDescriptorIndex, TypeAlias::ComPtr<ID3D12Resource2>& a_renderTargetResource, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool) const;

	private:

		bool IsValidGPUResourceReleaseRecord	(const GPUResourceReleaseRecord&     a_releaseRecord) const;
		bool IsValidDescriptorIndexReleaseRecord(const DescriptorIndexReleaseRecord& a_releaseRecord) const;

		void ReleaseAvailableGPUResources(const UINT64& a_completedFenceValue);

		template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
		void ReleaseAvailableDescriptorIndices(const UINT64& a_completedFenceValue, std::vector<DescriptorIndexReleaseRecord>& a_releaseRecordList, DescriptorPool<HeapType>& a_descriptorPool)
		{
			std::size_t l_index = 0ULL;

			while (l_index < a_releaseRecordList.size())
			{
				// GPUのフェンス値よりも大きいフェンス値ならまだ解放しない
				if (a_releaseRecordList[l_index].m_retiredFenceValue > a_completedFenceValue)
				{
					++l_index;

					continue;
				}

				// 該当するディスクリプタヒープから解放する
				if (a_releaseRecordList[l_index].m_descriptorIndex != DescriptorHeap::k_invalidDescriptorIndex)
				{
					a_descriptorPool.Release(a_releaseRecordList[l_index].m_descriptorIndex);
				}

				// 解放順は不要なので、末尾要素を移動してpop_backする(要素削除時に動作が速いため)
				// 要素を進めないことでswap後の末尾の要素も確認できるため。
				std::swap				    (a_releaseRecordList[l_index], a_releaseRecordList.back());
				a_releaseRecordList.pop_back();
			}
		}

		std::vector<GPUResourceReleaseRecord> m_gpuResourceReleaseRecordList = {};

		std::vector<DescriptorIndexReleaseRecord> m_rtvDescriptorIndexReleaseRecordList       = {};
		std::vector<DescriptorIndexReleaseRecord> m_cbvSRVUAVDescriptorIndexReleaseRecordList = {};
		std::vector<DescriptorIndexReleaseRecord> m_dsvDescriptorIndexReleaseRecordList       = {};
	};
}