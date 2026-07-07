#include "CopyCommandList.h"

void FWK::Graphics::CopyCommandList::Reset(const TypeAlias::CopyCommandAllocator& a_copyCommandAllocator)
{
	CommandListBase::Reset(a_copyCommandAllocator);
}

void FWK::Graphics::CopyCommandList::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION& a_destination,
													   const D3D12_TEXTURE_COPY_LOCATION& a_source,
													   const D3D12_BOX*					  a_sourceBox, 
													   const UINT						  a_destinationX,
													   const UINT						  a_destinationY,
													   const UINT						  a_destinationZ) const
{
	const auto& l_copyCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF(!l_copyCommandList, "コピーコマンドリストが作成されておらず、テクスチャコピー処理に失敗しました。");

	// D3D12_TEXTURE_COPY_LOCATION構造体についての説明
	// pResource : コピー先またはコピー元になるリソース
	// Type      : どの方法でコピー位置を指定するか
	//			　 SUBRESOURCE_INDEXなら「何番目のサブリソースか」
	//			   PLACED_FOOTPRINTなら「UploadBuffer上のどこに画像があるか」
	// どのリソースのどこへ、どのリソースのどこからコピーするかを指定する
	// CopyTextureRegion(コピー先の情報、
	//					 コピー先X座標、
	//					 コピー先Y座標、
	//					 コピー先Z座標,
	//					 コピー元の情報、
	//					 コピー元範囲(nullptrなら全体));
	l_copyCommandList->CopyTextureRegion(&a_destination,
										 a_destinationX,
										 a_destinationY,
										 a_destinationZ,
										 &a_source,
										 a_sourceBox);
}

void FWK::Graphics::CopyCommandList::CopyBufferRegion(const UINT64&			 a_destinationOffset, 
												      const UINT64&			 a_sourceOffset,
													  const UINT64&			 a_bufferSize,
	                                                        ID3D12Resource2& a_destinationBuffer,
	                                                        ID3D12Resource2& a_sourceBuffer) const
{
	const auto& l_copyCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF(!l_copyCommandList,						        "コピーコマンドリストが作成されておらず、バッファコピー処理に失敗しました。");
	FWK_ASSERT_RETURN_IF(a_bufferSize == Constant::k_invalidBufferSize, "コピーするBufferサイズが0のため、バッファコピー処理に失敗しました。");

	// CopyBufferRegion(コピー先BufferResource、
	//					コピー先BufferResource内の書き込み開始Offset、
	//					コピー元UploadBuffer、
	//					コピー元UploadBuffer内の読み取り開始Offset、
	//					コピーするByteSize);
	l_copyCommandList->CopyBufferRegion(&a_destinationBuffer,
										a_destinationOffset,
										&a_sourceBuffer,
										a_sourceOffset,
										a_bufferSize);
}