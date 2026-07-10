#include "SpriteScreenPerObjectDrawRequest.h"

void FWK::Graphics::SpriteScreenPerObjectDrawRequest::BeginFrame()
{
	// 前フレームのSprite描画申請を消す
	m_drawRequestDataList.BeginFrame();
}

void FWK::Graphics::SpriteScreenPerObjectDrawRequest::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource)
{
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	// PrimitiveTopologyTypeをセット
	l_directCommandList.SetupPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	for (const auto& l_drawRequestData : m_drawRequestDataList.GetREFArrayElementDataList())
	{
		const auto& l_drawRequest = l_drawRequestData.m_type.lock();

		if (!l_drawRequest) { continue; }

		Struct::CBSpritePerObject l_cbSpritePerObject = {};

		// TextureRecordからテクスチャのSRVIndexを取得
		const auto l_textureSRVIndex = FetchTextureSRVDescriptorIndex(l_drawRequest->m_textureRecord);

		l_cbSpritePerObject.m_baseColorTextureSRVIndex = l_textureSRVIndex;

		l_cbSpritePerObject.m_color		 = l_drawRequest->m_color;
		l_cbSpritePerObject.m_position	 = l_drawRequest->m_position;
		l_cbSpritePerObject.m_scale      = l_drawRequest->m_scale;
		l_cbSpritePerObject.m_pivot		 = l_drawRequest->m_pivot;
		l_cbSpritePerObject.m_sourceRECT = l_drawRequest->m_sourceRECT;

		SetupConstantBuffer<SpriteScreenPerObjectDynamicConstantBufferUploader>(l_cbSpritePerObject,
																		        a_rootSignature,
																		        l_directCommandList,
																		        a_frameResource,
																		        Enum::RootParameterType::CBSpritePerObject);

		l_directCommandList.DrawInstanced(Constant::k_spriteVertexCount,
										  Constant::k_spriteDrawInstanceCount,
										  Constant::k_spriteStartVertexLocation,
										  Constant::k_spriteStartInstanceLocation);
	}
}

void FWK::Graphics::SpriteScreenPerObjectDrawRequest::AddDrawRequestPerObject(const std::shared_ptr<DrawRequestData>&a_drawRequestData)
{
	m_drawRequestDataList.Add(a_drawRequestData);
}