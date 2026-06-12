#include "SpriteScreenPerObjectDrawRequest.h"

void FWK::Graphics::SpriteScreenPerObjectDrawRequest::BeginFrame()
{
	// 前フレームのSprite描画申請を消す
	m_drawRequestPerObjectList.BeginFrame();
}

void FWK::Graphics::SpriteScreenPerObjectDrawRequest::RequestDraw(const ResourceContext& a_resourceContext, const Renderer& a_renderer)
{
	const auto& l_srvDescriptorPool = a_resourceContext.GetREFSRVDescriptorPool();
	const auto& l_renderGraph       = a_renderer.GetREFRenderGraph			   ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList	   ();

	SetupSRVDescriptorHeap(l_directCommandList, l_srvDescriptorPool);

	// パイプラインステート、ルートシグネチャをセット
	const auto& l_rootSignature = SetupRenderPipeline(a_renderer, Enum::PipelineStateType::SpriteStandard).lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_rootSignature, "Forward描画用RootSignatureが無効なため、Sprite描画申請処理に失敗しました。");

	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "現在のフレームリソースの取得に失敗しており、Sprite描画申請処理に失敗しました。");

	const auto& l_spritePassDrawRequest = l_renderGraph.FindVALDrawRequestPass<SpriteScreenPassDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_spritePassDrawRequest,																						   "スプライトパスのポインタが無効になっており、Sprite描画申請処理に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_spritePassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "スプライト定数の設定が出来ませんでした、Sprite描画申請処理に失敗しました。");
	
	const auto& l_textureSystem = a_resourceContext.GetREFTextureSystem();

	for (const auto& l_drawRequest : m_drawRequestPerObjectList.GetREFDrawRequestPerObjectRecordList())
	{
		const auto& l_drawRequestPerObject = l_drawRequest.m_drawRequestPerObject.lock();

		if (!l_drawRequestPerObject) { continue; }

		Struct::CBSpritePerObject l_cbSpritePerObject = {};

		// TextureRecordが存在しなければデフォルトテクスチャを使用するため問題ない
		const auto& l_textureRecord   = l_drawRequestPerObject->m_textureRecord.lock();
		const auto  l_textureSRVIndex = FetchVALTextureSRVDescriptorIndex(l_textureRecord, l_textureSystem, Enum::DefaultTextureType::BaseColor);

		FWK_ASSERT_RETURN_IF_FAILED(l_textureSRVIndex == Constant::k_invalidDescriptorIndex, "BaseColorTextureのDescriptorIndexが無効になっており、Sprite描画申請処理に失敗しました。");

		l_cbSpritePerObject.m_baseColorTextureSRVIndex = l_textureSRVIndex;

		// もしTextureRecordがnullptrだったら、使用されるテクスチャは必ずデフォルトテクスチャになるので
		// フォールバックテクスチャとしてわかりやすいように256 * 256のスケールになるように拡大率を調整
		if (!l_textureRecord)
		{
			l_cbSpritePerObject.m_scale = k_defaultTextureScale;
		}
		else
		{
			l_cbSpritePerObject.m_scale = l_drawRequestPerObject->m_scale;
		}

		l_cbSpritePerObject.m_color		 = l_drawRequestPerObject->m_color;
		l_cbSpritePerObject.m_position	 = l_drawRequestPerObject->m_position;
		l_cbSpritePerObject.m_pivot		 = l_drawRequestPerObject->m_pivot;
		l_cbSpritePerObject.m_sourceRECT = l_drawRequestPerObject->m_sourceRECT;

		SetupPerObjectConstantBuffer<SpriteScreenPerObjectConstantBufferUploader>(*l_rootSignature,
																				  l_directCommandList,
																				  *l_currentFrameResource,
																				  l_cbSpritePerObject,
																				  Enum::RootParameterType::CBSpritePerObject);

		// MeshShaderを1グループ実行して、画面スプライト用の四角形を描画する。
		l_directCommandList.DispatchMesh(GetVALDefaultDispatchMeshThreadGroupCountX(), GetVALDefaultDispatchMeshThreadGroupCountY(), GetVALDefaultDispatchMeshThreadGroupCountZ());
	}
}

void FWK::Graphics::SpriteScreenPerObjectDrawRequest::AddDrawRequestPerObject(const std::shared_ptr<Struct::SpriteScreenPerObjectDrawRequestData>&a_drawRequestData)
{
	m_drawRequestPerObjectList.AddDrawRequestPerObject(a_drawRequestData);
}