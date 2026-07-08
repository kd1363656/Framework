#pragma once

namespace FWK::Graphics
{
	class StructuredBuffer final
	{
	public:

		 StructuredBuffer();
		~StructuredBuffer();

		StructuredBuffer(const StructuredBuffer&) = delete;
		StructuredBuffer(	   StructuredBuffer&& a_other) noexcept;

		StructuredBuffer& operator=(const StructuredBuffer&) = delete;
		StructuredBuffer& operator=(	  StructuredBuffer&& a_other) noexcept;

		template <typename Type>
		bool Create(const std::vector<Type>&                        a_bufferList, 
					const Device&                                   a_device,
					const GPUMemoryAllocator&                       a_gpuMemoryAllocator,
						  std::vector<Struct::BufferUploadCommand>& a_bufferUploadCommandList,
						  TypeAlias::CBVSRVUAVDescriptorPool&       a_cbvSRVUAVDescriptorPool)
		{
			// ストラクチャードバッファーを作成するための条件がそろっているのかどうかを確認する
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferList.empty(),                                       "BufferListが空のため、StructuredBufferの作成に失敗しました。",                                    false);
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferList.size() > k_maxStructuredBufferElementCount,    "StructuredBufferの要素数がUINTの最大値を超えたため、StructuredBufferの作成に失敗しました。",      false);
			FWK_ASSERT_RETURN_VALUE_IF(sizeof(Type) > std::numeric_limits<UINT>::max(),            "StructuredBufferの1要素サイズがUINTの最大値を超えたため、StructuredBufferの作成に失敗しました。", false);
			FWK_ASSERT_RETURN_VALUE_IF(m_bufferGPUResource.m_resource,			                   "StructuredBufferは既にGPUResourceを保持しているため、再作成できません。",                         false);
			FWK_ASSERT_RETURN_VALUE_IF(m_srvDescriptorIndex != Constant::k_invalidDescriptorIndex, "StructuredBufferは既にSRVDescriptorIndexを保持しているため、再作成できません。",                  false);

			const auto& l_bufferSize = static_cast<UINT64>(sizeof(Type)) * static_cast<UINT64>(a_bufferList.size());

			// バッファーサイズがUINTが保持できる上限値を超えているかどうかを確認する
			FWK_ASSERT_RETURN_VALUE_IF(l_bufferSize == Constant::k_invalidBufferSize, "StructuredBufferの作成サイズが0のため、StructuredBufferの作成に失敗しました。", false);

			// 失敗しても、このStructuredBufferが中途半端な状態にならないように、まずはローカル変数で作る
			Struct::GPUResource l_bufferGPUResource = {};

			// リソース作成のためのメモリ領域を確保
			FWK_ASSERT_RETURN_VALUE_IF(!a_gpuMemoryAllocator.CreateBufferResource(l_bufferSize, D3D12_RESOURCE_STATE_COMMON, l_bufferGPUResource), "StructuredBuffer用GPUResourceの作成に失敗しました。", false);

			Struct::BufferUploadCommand l_bufferUploadCommand = {};

			// バッファーのアップロード先のデフォルトヒープにリソースを作成
			FWK_ASSERT_RETURN_VALUE_IF(!CreateBufferUploadCommand(a_bufferList,
																  a_device,
																  l_bufferGPUResource,
																  l_bufferSize,
																  l_bufferUploadCommand),
																  "StructuredBuffer用UploadCommandの作成に失敗しました。",
																  false);

			// ストラクチャードバッファー用のSRVを作成
			const auto l_srvDescriptorIndex = CreateStructuredBufferSRV(a_bufferList,
																		a_device,
																		l_bufferGPUResource,
																		a_cbvSRVUAVDescriptorPool);

			FWK_ASSERT_RETURN_VALUE_IF(l_srvDescriptorIndex == Constant::k_invalidDescriptorIndex, "StructuredBuffer用SRVの作成に失敗しました。", false);

			// 作成内容をメンバに反映
			m_bufferGPUResource  = std::move(l_bufferGPUResource);
			m_srvDescriptorIndex = l_srvDescriptorIndex;

			a_bufferUploadCommandList.emplace_back(std::move(l_bufferUploadCommand));

			return true;
		}
		
		bool ReserveRelease                      (const UINT64&					      a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext);
		void ReleaseImmediatelySRVDescriptorIndex(TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);
		void Release();

		const auto& GetREFBufferGPUResource() const { return m_bufferGPUResource; }

		auto GetVALSRVDescriptorIndex() const { return m_srvDescriptorIndex; }

	private:

		template <typename Type>
		bool CreateBufferUploadCommand(const std::vector<Type>&           a_bufferList, 
									   const Device&                      a_device,
									   const Struct::GPUResource&         a_bufferGPUResource,
									   const UINT64&					  a_bufferSize,
									   	     Struct::BufferUploadCommand& a_bufferUploadCommand)
		{
			FWK_ASSERT_RETURN_VALUE_IF(!a_bufferGPUResource.m_resource,               "StructuredBuffer用GPUResourceが無効のため、UploadCommandの作成に失敗しました。", false);
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferList.empty(),                          "BufferListが空のため、UploadCommandの作成に失敗しました。",					    false);
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferSize == Constant::k_invalidBufferSize, "BufferSizeが0のため、UploadCommandの作成に失敗しました。",					    false);

			// 作成し終わったデフォルトヒープ上にあるリソースをコピー先として扱う
			a_bufferUploadCommand.m_destinationBufferResource = a_bufferGPUResource.m_resource;

			auto& l_bufferUploadRecord = a_bufferUploadCommand.m_bufferUploadRecord;

			l_bufferUploadRecord.m_bufferSize = a_bufferSize;

			// デフォルトヒープにコピーするためのアップロードヒープを作成する
			FWK_ASSERT_RETURN_VALUE_IF(!l_bufferUploadRecord.m_uploadBuffer.Create(a_device, a_bufferSize), "StructuredBuffer用UploadBufferの作成に失敗しました。", false);

			auto* l_mappedData = l_bufferUploadRecord.m_uploadBuffer.FetchPTRMappedData();

			FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "StructuredBuffer用UploadBufferのMap済みデータ取得に失敗しました。", false);

			// デフォルトヒープコピー用のアップロードヒープにデータを送信
			std::memcpy(l_mappedData, a_bufferList.data(), a_bufferSize);

			return true;
		}

		template <typename Type>
		TypeAlias::DescriptorIndex CreateStructuredBufferSRV(const std::vector<Type>&			       a_bufferList,
															 const Device&						       a_device,
															 const Struct::GPUResource&			       a_bufferGPUResource,
																   TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool) const
		{
			const auto& l_device = a_device.GetREFDevice();

			FWK_ASSERT_RETURN_VALUE_IF(!l_device,												"デバイスが作成されておらず、StructuredBuffer用SRVの作成に失敗しました。",                         Constant::k_invalidDescriptorIndex);
			FWK_ASSERT_RETURN_VALUE_IF(!a_bufferGPUResource.m_resource,						    "BufferResourceが無効のため、StructuredBuffer用SRVの作成に失敗しました。",                         Constant::k_invalidDescriptorIndex);
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferList.empty(),									    "BufferListが空のため、StructuredBuffer用SRVの作成に失敗しました。",                               Constant::k_invalidDescriptorIndex);
			FWK_ASSERT_RETURN_VALUE_IF(a_bufferList.size() > k_maxStructuredBufferElementCount, "StructuredBufferの要素数がUINTの最大値を超えたため、StructuredBuffer用SRVの作成に失敗しました。", Constant::k_invalidDescriptorIndex);

			const auto l_srvDescriptorIndex = a_cbvSRVUAVDescriptorPool.Allocate();

			FWK_ASSERT_RETURN_VALUE_IF(l_srvDescriptorIndex == Constant::k_invalidDescriptorIndex, "SRV用DescriptorIndexの確保に失敗したため、StructuredBuffer用のSRVの作成に失敗しました。", Constant::k_invalidDescriptorIndex);

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

			if (l_srvDescriptorIndex == Constant::k_invalidDescriptorIndex)
			{
				a_srvDescriptorPool.Release(l_srvDescriptorIndex);
				FWK_ASSERT_RETURN_VALUE	   ("StructuredBuffer用SRVの作成に失敗しました。", Constant::k_invalidDescriptorIndex);
			}

			return l_srvDescriptorIndex;
		}

		void MoveFrom(StructuredBuffer&& a_other) noexcept;

		static constexpr UINT64 k_firstStructuredBufferElement    = 0ULL;
		static constexpr UINT64 k_maxStructuredBufferElementCount = std::numeric_limits<UINT>::max();
		
		Struct::GPUResource m_bufferGPUResource;

		TypeAlias::DescriptorIndex m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;
	};
}