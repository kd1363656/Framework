#pragma once

namespace FWK::Graphics
{
	class DirectCommandQueue;
}

namespace FWK::Graphics
{
	class ResourceReleaseContext
	{
	public:

		bool PushSRVDescriptorIndex(Struct::DescriptorIndexReleaseRecord&& a_releaseRecord);

		void ReleaseAvailableDeferredResources(const DirectCommandQueue& a_directCommandQueue, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool);

		// ※ 注意 この関数を呼び出す前に必ずGPUとの同期をとること
		TypeAlias::DescriptorIndex ReleaseRenderTargetResourceImmediately(const TypeAlias::DescriptorIndex a_rtvDescriptorIndex, TypeAlias::ComPtr<ID3D12Resource2>& a_renderTargetResource, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool) const;

	private:

		bool IsValidDescriptorIndexReleaseRecord(const Struct::DescriptorIndexReleaseRecord& a_releaseRecord) const;

		template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
		void ReleaseAvailableDescriptorIndices(const UINT64& a_completedFenceValue, std::vector<Struct::DescriptorIndexReleaseRecord>& a_releaseRecordList, DescriptorPool<HeapType>& a_descriptorPool)
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
				if (a_releaseRecordList[l_index].m_descriptorIndex != Constant::k_invalidDescriptorIndex)
				{
					a_descriptorPool.Release(a_releaseRecordList[l_index].m_descriptorIndex);
				}

				// 解放順は不要なので、末尾要素を移動してpop_backする(要素削除時に動作が速いため)
				// 要素を進めないことでswap後の末尾の要素も確認できるため。
				std::swap(a_releaseRecordList[l_index], a_releaseRecordList.back());
				a_releaseRecordList.pop_back();
			}
		}

		std::vector<Struct::DescriptorIndexReleaseRecord> m_srvDescriptorIndexReleaseRecordList = {};
	};
}