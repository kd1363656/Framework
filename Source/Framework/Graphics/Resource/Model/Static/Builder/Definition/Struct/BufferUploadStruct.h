#pragma once

namespace FWK::Struct
{
	struct BufferUploadRecord final
	{
		// DEFAULTヒープ上のBufferResourceへコピーするための中間バッファ
		Graphics::UploadBuffer m_uploadBuffer = {};

		// コピーするバッファサイズ
		UINT64 m_bufferSize = Constant::k_invalidBufferSize;
	};

	struct BufferUploadCommand final
	{
		// バッファーへコピーするためのUpload情報
		BufferUploadRecord m_bufferUploadRecord = {};

		// Upload先のDEFAULTヒープ上
		// CopyBufferRegionではAllocationを使用しないため、GPUResource全体ではなくD3D12Resource2のみを保存する
		TypeAlias::ComPtr<ID3D12Resource2> m_destinationBufferResource = nullptr;
	};
}