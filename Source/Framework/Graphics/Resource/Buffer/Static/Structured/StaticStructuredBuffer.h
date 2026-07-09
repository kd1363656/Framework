#pragma once

namespace FWK::Graphics
{
	class StaticStructuredBuffer final : public StructuredBufferBase
	{
	public:

		struct BufferUploadRecord final
		{
			// DEFAULTヒープ上のBufferResourceへコピーするための中間バッファ
			Graphics::UploadBuffer m_uploadBuffer = {};

			// コピーするバッファサイズ
			UINT64 m_bufferSize = Graphics::UploadBuffer::k_invalidBufferSize;
		};

		struct BufferUploadCommand final
		{
			// バッファーへコピーするためのUpload情報
			BufferUploadRecord m_bufferUploadRecord = {};

			// Upload先のDEFAULTヒープ上
			// CopyBufferRegionではAllocationを使用しないため、GPUResource全体ではなくD3D12Resource2のみを保存する
			TypeAlias::ComPtr<ID3D12Resource2> m_destinationBufferResource = nullptr;
		};

	public:

		 StaticStructuredBuffer();
		~StaticStructuredBuffer() override;

		StaticStructuredBuffer(const StaticStructuredBuffer&) = delete;
		StaticStructuredBuffer(	     StaticStructuredBuffer&& a_other) noexcept;

		StaticStructuredBuffer& operator=(const StaticStructuredBuffer&) = delete;
		StaticStructuredBuffer& operator=(	    StaticStructuredBuffer&& a_other) noexcept;

		template <typename Type>
		bool Create(const std::vector<Type>&                  a_bufferList, 
					const Device&                             a_device,
					const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
						  std::vector<BufferUploadCommand>&   a_bufferUploadCommandList,
						  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
		{
			// ストラクチャードバッファーを作成するための条件がそろっているのかどうかを確認する
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferList.empty(),                                                   "BufferListが空のため、StaticStructuredBufferの作成に失敗しました。",                                          false);
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferList.size() > k_maxStructuredBufferElementCount,                "StaticStructuredBufferの要素数がUINTの最大値を超えたため、StaticStructuredBufferの作成に失敗しました。",      false);
			FWK_ASSERT_RETURN_VALUE_IF(sizeof(Type) > std::numeric_limits<UINT>::max(),                        "StaticStructuredBufferの1要素サイズがUINTの最大値を超えたため、StaticStructuredBufferの作成に失敗しました。", false);
			FWK_ASSERT_RETURN_VALUE_IF(GetREFBufferGPUResource().m_resource,                                   "StaticStructuredBufferは既にGPUResourceを保持しているため、再作成できません。",                               false);
			FWK_ASSERT_RETURN_VALUE_IF(GetVALSRVDescriptorIndex() != DescriptorHeap::k_invalidDescriptorIndex, "StaticStructuredBufferは既にSRVDescriptorIndexを保持しているため、再作成できません。",                        false);

			const auto& l_bufferSize = static_cast<UINT64>(sizeof(Type)) * static_cast<UINT64>(a_bufferList.size());

			// バッファーサイズがUINTが保持できる上限値を超えているかどうかを確認する
			FWK_ASSERT_RETURN_VALUE_IF(l_bufferSize == UploadBuffer::k_invalidBufferSize, "StaticStructuredBufferの作成サイズが0のため、StaticStructuredBufferの作成に失敗しました。", false);

			// 失敗しても、このStaticStructuredBufferが中途半端な状態にならないように、まずはローカル変数で作る
			ResourceReleaseContext::GPUResource l_bufferGPUResource = {};

			// リソース作成のためのメモリ領域を確保
			FWK_ASSERT_RETURN_VALUE_IF(!a_gpuMemoryAllocator.CreateBufferResource(l_bufferSize, 
																				  D3D12_RESOURCE_FLAG_NONE,
																				  D3D12_RESOURCE_STATE_COMMON,
																				  l_bufferGPUResource), 
																				  "StaticStructuredBuffer用GPUResourceの作成に失敗しました。", 
																				  false);

			BufferUploadCommand l_bufferUploadCommand = {};

			// バッファーのアップロード先のデフォルトヒープにリソースを作成
			FWK_ASSERT_RETURN_VALUE_IF(!CreateBufferUploadCommand(a_bufferList,
																  a_device,
																  l_bufferGPUResource,
																  l_bufferSize,
																  l_bufferUploadCommand),
																  "StaticStructuredBuffer用UploadCommandの作成に失敗しました。",
																  false);

			// ストラクチャードバッファー用のSRVを作成
			const auto l_srvDescriptorIndex = CreateSRV(a_device,
				                                        l_bufferGPUResource,
														static_cast<UINT>(a_bufferList.size()),
														static_cast<UINT>(sizeof(Type)),
														a_cbvSRVUAVDescriptorPool);

			FWK_ASSERT_RETURN_VALUE_IF(l_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "StaticStructuredBuffer用SRVの作成に失敗しました。", false);

			// 作成内容をメンバに反映
			SetBufferGPUResource (std::move(l_bufferGPUResource));
			SetSRVDescriptorIndex(l_srvDescriptorIndex);

			a_bufferUploadCommandList.emplace_back(std::move(l_bufferUploadCommand));

			return true;
		}
		
		bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;
		void Release       ()                                                                                    override;

	private:

		template <typename Type>
		bool CreateBufferUploadCommand(const std::vector<Type>&                   a_bufferList, 
									   const Device&                              a_device,
									   const ResourceReleaseContext::GPUResource& a_bufferGPUResource,
									   const UINT64&					          a_bufferSize,
									   	     BufferUploadCommand&                 a_bufferUploadCommand)
		{
			FWK_ASSERT_RETURN_VALUE_IF(!a_bufferGPUResource.m_resource,                   "StaticStructuredBuffer用GPUResourceが無効のため、UploadCommandの作成に失敗しました。", false);
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferList.empty(),                              "BufferListが空のため、UploadCommandの作成に失敗しました。",					          false);
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferSize == UploadBuffer::k_invalidBufferSize, "BufferSizeが0のため、UploadCommandの作成に失敗しました。",					          false);

			// 作成し終わったデフォルトヒープ上にあるリソースをコピー先として扱う
			a_bufferUploadCommand.m_destinationBufferResource = a_bufferGPUResource.m_resource;

			auto& l_bufferUploadRecord = a_bufferUploadCommand.m_bufferUploadRecord;

			l_bufferUploadRecord.m_bufferSize = a_bufferSize;

			// デフォルトヒープにコピーするためのアップロードヒープを作成する
			FWK_ASSERT_RETURN_VALUE_IF(!l_bufferUploadRecord.m_uploadBuffer.Create(a_device, a_bufferSize), "StaticStructuredBuffer用UploadBufferの作成に失敗しました。", false);

			auto* l_mappedData = l_bufferUploadRecord.m_uploadBuffer.FetchPTRMappedData();

			FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "StaticStructuredBuffer用UploadBufferのMap済みデータ取得に失敗しました。", false);

			// デフォルトヒープコピー用のアップロードヒープにデータを送信
			std::memcpy(l_mappedData, a_bufferList.data(), a_bufferSize);

			return true;
		}	
	};
}