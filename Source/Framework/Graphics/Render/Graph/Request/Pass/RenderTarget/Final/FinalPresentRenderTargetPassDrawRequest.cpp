#include "FinalPresentRenderTargetPassDrawRequest.h"

bool FWK::Graphics::FinalPresentRenderTargetPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	// FinalPresentPassでは、PhysicsDebugPassまで書き込み済みのFinalColorをSRVとして読んでBackBufferへ書き込む
	const auto& l_finalColorRenderTargetPassTexture = FetchVALRenderTargetPassTexture(a_frameResource).lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_finalColorRenderTargetPassTexture, "FinalColorのRenderTargetPassTextureが無効のため、FinalPresentPass定数の設定に失敗しました。", false);

	const auto& l_renderTargetTexture        = l_finalColorRenderTargetPassTexture->GetREFRenderTargetTexture();
		  auto& l_finalPresentConstantBuffer = GetMutableREFConstantBuffer                                   ();

	// 定数バッファにシーンカラーテクスチャのSRVDescriptorIndexを渡す
	l_finalPresentConstantBuffer.m_sceneColorTextureSRVIndex = l_renderTargetTexture.GetVALSRVDescriptorIndex();

	// 定数バッファをセット
	SetupConstantBuffer<FinalPresentRenderTargetPassDynamicConstantBufferUploader>(l_finalPresentConstantBuffer,
																		           a_rootSignature,
																		           a_directCommandList,
																		           a_frameResource,
																		           Enum::RootParameterType::CBFinalColorPass);

	// 定数バッファーをセットしたらPrimitiveTopologyを設定して描画申請
	a_directCommandList.SetupPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// SceneColor全体を読み、FinalColorへ全画面描画する
	a_directCommandList.DrawInstanced(Constant::k_triangleVertexCount,
									  Constant::k_defaultTriangleInstanceCount,
									  Constant::k_defaultTriangleStartVertexLocation,
									  Constant::k_defaultTriangleStartInstanceLocation);

	return true;
}