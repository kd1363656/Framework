#include "CopyCommandList.h"

FWK::Graphics::CopyCommandList::CopyCommandList() : 
	CommandListBase(Constant::k_createCommandListTypeCopy)
{}
FWK::Graphics::CopyCommandList::~CopyCommandList() = default;

void FWK::Graphics::CopyCommandList::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION& a_destination, 
													   const D3D12_TEXTURE_COPY_LOCATION& a_source,
													   const D3D12_BOX*					  a_sourceBox, 
													   const UINT						  a_destinationX,
													   const UINT						  a_destinationY,
													   const UINT						  a_destinationZ) const
{
	const auto& l_copyCommandList = GetREFCommandList();

	FWK_ASSERT_RETURN_IF_FAILED(!l_copyCommandList, "コピーコマンドリストが作成されておらず、テクスチャコピー処理に失敗しました。");

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