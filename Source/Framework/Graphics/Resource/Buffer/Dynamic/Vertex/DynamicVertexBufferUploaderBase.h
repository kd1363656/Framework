#pragma once

namespace FWK::Graphics
{
	template <typename VertexType>
	class DynamicVertexBufferUploaderBase : public DynamicBufferUploaderBase
	{
	public:

		DynamicVertexBufferUploaderBase() : 
			DynamicBufferUploaderBase(sizeof(VertexType))
		{}
		~DynamicVertexBufferUploaderBase() override = default;

		bool Create(const Device& a_device) override
		{
			// 頂点バッファのAlignmentは定数バッファのような256バイトアライメント制約を持たない、
			// uint32_t固定なので、1要素4バイト単位で連続配置する。
			FWK_ASSERT_RETURN_VALUE_IF(!CreateUploadBuffer(a_device, sizeof(VertexType)), "DynamicVertexBufferUploader用UploadBufferの作成に失敗しました。", false);

			return true;
		}

		D3D12_VERTEX_BUFFER_VIEW WriteVertexList(const std::vector<VertexType>& a_vertexList)
		{
			FWK_ASSERT_RETURN_VALUE_IF(a_vertexList.empty(), "VertexListの要素数が空のため、DynamicVertexBufferUploaderの書き込みに失敗しました。", {});
			
			const auto& l_gpuVirtualAddress = WriteContiguousElementListAndAdvance(a_vertexList);

			FWK_ASSERT_RETURN_VALUE_IF(l_gpuVirtualAddress == GetREFInvalidGPUVirtualAddress(), "VertexListの書き込みに失敗したため、VertexBufferViewの作成に失敗しました。", {});

			const auto l_vertexBufferSize = sizeof(VertexType) * a_vertexList.size();

			D3D12_VERTEX_BUFFER_VIEW l_vertexBufferView = {};

			l_vertexBufferView.BufferLocation = l_gpuVirtualAddress;
			l_vertexBufferView.SizeInBytes    = static_cast<UINT>(l_vertexBufferSize);
			l_vertexBufferView.StrideInBytes  = sizeof           (VertexType);

			return l_vertexBufferView;
		}
	};
}