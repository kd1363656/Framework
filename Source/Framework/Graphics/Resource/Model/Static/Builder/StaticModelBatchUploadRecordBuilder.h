#pragma once

namespace FWK::Graphics
{
	class StaticModelBatchUploadRecordBuilder final
	{
	public:

		 StaticModelBatchUploadRecordBuilder() = default;
		~StaticModelBatchUploadRecordBuilder() = default;

		bool CreateStaticModelBatchUploadRecord(const Device&									a_device,
												const GPUMemoryAllocator&					    a_gpuMemoryAllocator,
													  std::vector<Struct::BufferUploadCommand>& a_bufferUploadCommandList,
													  TypeAlias::SRVDescriptorPool&		        a_srvDescriptorPool,
													  Graphics::StaticModelRecord&			    a_staticModelRecord) const;

	private:

		bool CreateModelBatchUploadRecord(const Device&									  a_device,
										  const GPUMemoryAllocator&						  a_gpuMemoryAllocator,
											    std::vector<Struct::BufferUploadCommand>& a_bufferUploadCommandList,
												TypeAlias::SRVDescriptorPool&		      a_srvDescriptorPool,
												Struct::StaticModelMesh&				  a_staticModelMesh) const;

		template <typename Type>
		bool CreateBufferUploadCommand(const std::vector<Type>&                        a_bufferList,
									   const Device&			                       a_device,
									   const GPUMemoryAllocator&                       a_gpuMemoryAllocator,
										     std::vector<Struct::BufferUploadCommand>& a_bufferUploadCommandList,
											 Struct::GPUResource&					   a_destinationBufferGPUResource) const
		{
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_bufferList.empty(), "BufferListが空のため、BufferUploadCommandの作成に失敗しました。", false);

			// 型 * リストのサイズ分で総サイズを取得してバッファー作成の時に渡すバッファーサイズとして扱う
			const auto& l_bufferSize = sizeof(Type) * a_bufferList.size();

			// DEFAULTヒープ上に、MeshShaderから参照する本番用BufferResourceを作成する
			// 初期状態をCOMMONにしておくことでCopyQueue上のCopyBufferRegion時にCOPY_DESTへ暗黙的に昇格できる
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_gpuMemoryAllocator.CreateBufferResource(l_bufferSize, D3D12_RESOURCE_STATE_COMMON, a_destinationBufferGPUResource), "StaticModel用BufferResourceの作成に失敗したため、BufferUploadCommandの作成に失敗しました。", false);

			Struct::BufferUploadCommand l_bufferUploadCommand = {};

			l_bufferUploadCommand.m_destinationBufferResource = a_destinationBufferGPUResource.m_resource;
			
			auto& l_bufferUploadRecord = l_bufferUploadCommand.m_bufferUploadRecord;

			l_bufferUploadRecord.m_bufferSize = l_bufferSize;

			// DEFAULTヒープ上のBufferResourceへコピーするためのUploadBufferを作成する
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_bufferUploadRecord.m_uploadBuffer.Create(a_device, l_bufferSize), "StaticModel用UploadBufferの作成に失敗したため、BufferUploadCommandの作成に失敗しました。", false);

			auto* l_mappedData = l_bufferUploadRecord.m_uploadBuffer.FetchPTRMappedData();

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_mappedData, "StaticModel用UploadBufferのMapに失敗したため、BufferUploadCommandの作成に失敗しました。", false);

			// バッファー内容をコピー
			std::memcpy(l_mappedData, a_bufferList.data(), l_bufferSize);

			// バッファーアップロード用リスト末尾に追加
			a_bufferUploadCommandList.emplace_back(std::move(l_bufferUploadCommand));

			return true;
		}

		void ReleaseCreatedStaticModelStructuredBufferSRV(std::vector<Struct::StaticModelMesh>& a_staticModelMeshList, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool) const;
	};
}