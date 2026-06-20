#include "SpriteScreenPerObjectDrawRequest.h"

void FWK::Graphics::SpriteScreenPerObjectDrawRequest::BeginFrame()
{
	// 前フレームのSprite描画申請を消す
	m_drawRequestPerObjectList.BeginFrame();
}

void FWK::Graphics::SpriteScreenPerObjectDrawRequest::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	for (const auto& l_drawRequest : m_drawRequestPerObjectList.GetREFDrawRequestPerObjectRecordList())
	{
		const auto& l_drawRequestPerObject = l_drawRequest.m_drawRequestPerObject.lock();

		if (!l_drawRequestPerObject) { continue; }

		Struct::CBSpritePerObject l_cbSpritePerObject = {};

		// TextureRecordからテクスチャのSRVIndexを取得
		const auto l_textureSRVIndex = FetchTextureSRVDescriptorIndex(l_drawRequestPerObject->m_textureRecord);

		l_cbSpritePerObject.m_baseColorTextureSRVIndex = l_textureSRVIndex;

		l_cbSpritePerObject.m_color		 = l_drawRequestPerObject->m_color;
		l_cbSpritePerObject.m_position	 = l_drawRequestPerObject->m_position;
		l_cbSpritePerObject.m_scale      = l_drawRequestPerObject->m_scale;
		l_cbSpritePerObject.m_pivot		 = l_drawRequestPerObject->m_pivot;
		l_cbSpritePerObject.m_sourceRECT = l_drawRequestPerObject->m_sourceRECT;

		SetupConstantBuffer<SpriteScreenPerObjectConstantBufferUploader>(l_cbSpritePerObject, 
																		 a_rootSignature,
																		 l_directCommandList,
																		 a_frameResource,
																		 Enum::RootParameterType::CBSpritePerObject);

		// MeshShaderを1グループ実行して、画面スプライト用の四角形を描画する。
		l_directCommandList.DispatchMesh(Constant::k_defaultDispatchMeshThreadGroupCountX, Constant::k_defaultDispatchMeshThreadGroupCountY, Constant::k_defaultDispatchMeshThreadGroupCountZ);
	}
}

void FWK::Graphics::SpriteScreenPerObjectDrawRequest::AddDrawRequestPerObject(const std::shared_ptr<Struct::SpriteScreenPerObjectDrawRequestData>&a_drawRequestData)
{
	m_drawRequestPerObjectList.AddDrawRequestPerObject(a_drawRequestData);
}