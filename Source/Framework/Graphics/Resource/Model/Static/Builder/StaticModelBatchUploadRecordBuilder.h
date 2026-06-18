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

		static constexpr UINT64 k_firstStructuredBufferElement = 0ULL;

		static constexpr UINT64 k_maxStructuredBufferElementCount = std::numeric_limits<UINT>::max();

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

		template <typename Type>
		TypeAlias::DescriptorIndex CreateStructuredBufferSRV(const std::vector<Type>&			 a_bufferList,
													         const Struct::GPUResource&			 a_bufferGPUResource,
													         const Device&						 a_device,
													         	   TypeAlias::SRVDescriptorPool& a_srvDescriptorPool) const
		{
			const auto& l_device = a_device.GetREFDevice();

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_device,												   "デバイスが作成されておらず、StructuredBuffer用SRVの作成に失敗しました。",						  Constant::k_invalidDescriptorIndex);
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_bufferGPUResource.m_resource,						   "BufferResourceが無効のため、StructuredBuffer用SRVの作成に失敗しました。",						  Constant::k_invalidDescriptorIndex);
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_bufferList.empty(),									   "BufferListが空のため、StructuredBuffer用SRVの作成に失敗しました。",								  Constant::k_invalidDescriptorIndex);
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_bufferList.size() > k_maxStructuredBufferElementCount, "StructuredBufferの要素数がUINTの最大値を超えたため、StructuredBuffer用SRVの作成に失敗しました。", Constant::k_invalidDescriptorIndex);

			const auto l_srvDescriptorIndex = a_srvDescriptorPool.Allocate();

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_srvDescriptorIndex == Constant::k_invalidDescriptorIndex, "SRV用DescriptorIndexの確保に失敗したため、StructuredBuffer用のSRVの作成に失敗しました。", Constant::k_invalidDescriptorIndex);

			D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};

			// D3D12_SHADER_RESOURCE_VIEW_DESCについて
			// Shader4ComponentMapping : Shader側でRGBA部分をどのように読むか
			// Format                  : StructuredBufferなのでDXGI_FORMAT_UNKNOWNを指定する
			// ViewDimension		   : BufferをSRVとして参照するためBUFFERを指定する
			l_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			l_srvDesc.Format				  = DXGI_FORMAT_UNKNOWN;
			l_srvDesc.ViewDimension			  = D3D12_SRV_DIMENSION_BUFFER;

			// D3D12_BUFFER_SRVについて
			// FirstElement         : 先頭要素番号
			// NumElements          : StructuredBufferとして参照する要素数
			// StructureByteStride  : 1要素あたりのバイトサイズ
			// Flags				: RawBufferではないためNONEを指定する
			l_srvDesc.Buffer.FirstElement        = k_firstStructuredBufferElement;
			l_srvDesc.Buffer.NumElements         = static_cast<UINT>(a_bufferList.size());
			l_srvDesc.Buffer.StructureByteStride = sizeof(Type);
			l_srvDesc.Buffer.Flags				 = D3D12_BUFFER_SRV_FLAG_NONE;

			const auto l_cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(a_srvDescriptorPool.FetchVALCPUDescriptorHandle(l_srvDescriptorIndex));

			// CreateShaderResourceView(BufferResource, 
			//							SRV設定、
			//							CPUOnlyDescriptorHeap側のCPUHandle);
			l_device->CreateShaderResourceView(a_bufferGPUResource.m_resource.Get(), &l_srvDesc, l_cpuHandle);

			if (!a_srvDescriptorPool.CopyCPUDescriptorToShaderVisibleDescriptor(a_device, l_srvDescriptorIndex))
			{
				a_srvDescriptorPool.Release(l_srvDescriptorIndex);

				FWK_ASSERT_RETURN_VALUE("CPUOnlyからShaderVisibleSRVへのコピーに失敗したため、StructuredBuffer用SRVの作成に失敗しました。", Constant::k_invalidDescriptorIndex);
			}

			return l_srvDescriptorIndex;
		}

		template <typename Type>
		void CreateStructuredBufferResource(const std::vector<Type>&			    a_bufferList,
											const Device&						    a_device,
												  TypeAlias::SRVDescriptorPool&     a_srvDescriptorPool,
												  Struct::StructuredBufferResource& a_structuredBufferResource) const
		{
			const auto  l_srvDescriptorIndex = CreateStructuredBufferSRV(a_bufferList,
																		 a_structuredBufferResource.m_bufferGPUResource,
																		 a_device,
																		 a_srvDescriptorPool);

			if (l_srvDescriptorIndex == Constant::k_invalidDescriptorIndex)
			{
				ReleaseCreatedStructuredBufferSRV(a_structuredBufferResource, a_srvDescriptorPool);
				FWK_ASSERT_RETURN			     ("StructuredBuffer用SRVの作成に失敗しました。");
			}

			a_structuredBufferResource.m_srvDescriptorIndex = l_srvDescriptorIndex;
		}

		void ReleaseCreatedStructuredBufferSRV           (Struct::StructuredBufferResource&     a_structuredBufferResource, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool) const;
		void ReleaseCreatedStaticModelStructuredBufferSRV(std::vector<Struct::StaticModelMesh>& a_staticModelMeshList,		TypeAlias::SRVDescriptorPool& a_srvDescriptorPool) const;
	};
}