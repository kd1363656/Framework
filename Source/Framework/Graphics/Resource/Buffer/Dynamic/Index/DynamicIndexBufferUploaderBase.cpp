#include "DynamicIndexBufferUploaderBase.h"

FWK::Graphics::DynamicIndexBufferUploaderBase::DynamicIndexBufferUploaderBase() : 
	DynamicBufferUploaderBase(sizeof(std::uint32_t))
{}

FWK::Graphics::DynamicIndexBufferUploaderBase::~DynamicIndexBufferUploaderBase()
{}

bool FWK::Graphics::DynamicIndexBufferUploaderBase::Create(const Device& a_device)
{
	// 頂点バッファのAlignmentは特に制約がない
	FWK_ASSERT_RETURN_VALUE_IF(!CreateUploadBuffer(a_device, sizeof(std::uint32_t)), "DynamicIndexBufferUploader用UploadBufferの作成に失敗しました。", false);

	return true;
}

D3D12_INDEX_BUFFER_VIEW FWK::Graphics::DynamicIndexBufferUploaderBase::WriteIndexList(const std::vector<std::uint32_t>& a_indexList)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_indexList.empty(), "IndexListの要素数がUINTの最大値を超えているため、DynamicIndexBufferUploaderの書き込みに失敗しました。", {});
	
	const auto& l_gpuVirtualAddress = WriteContiguousElementListAndAdvance(a_indexList);

	FWK_ASSERT_RETURN_VALUE_IF(l_gpuVirtualAddress == Constant::k_invalidGPUVirtualAddress, "IndexListの書き込みに失敗したため、IndexBufferViewの作成に失敗しました。", {});

	const auto l_indexBufferSize = sizeof(std::uint32_t) * a_indexList.size();

	D3D12_INDEX_BUFFER_VIEW l_indexBufferView = {};

	l_indexBufferView.BufferLocation = l_gpuVirtualAddress;
	l_indexBufferView.SizeInBytes    = static_cast<UINT>(l_indexBufferSize);
	l_indexBufferView.Format         = k_indexFormat;

	return l_indexBufferView;
}