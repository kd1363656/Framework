#include "Scene.h"

void FWK::Scene::INIT()
{
	if (!m_texture)
	{
		m_texture = std::make_shared<Graphics::Texture>();
	}

	if (!m_spriteDrawRequestData)
	{
		m_spriteDrawRequestData = std::make_shared<Struct::SpriteScreenPerObjectDrawRequestData>();
	}

	if (!m_staticModel)
	{
		m_staticModel = std::make_shared<Graphics::StaticModel>();
	}

	// テクスチャ
	const bool l_isLoadSuccess = m_texture->Load("Asset/Texture/Te.png", Enum::TextureLoadColorSpace::SRGB);

	m_spriteDrawRequestData->m_scale         = { 1.0F, 1.0F };
	m_spriteDrawRequestData->m_textureRecord = m_texture->GetREFTextureRecord();
	m_spriteDrawRequestData->m_sourceRECT    = { 0U, 0U, 256U, 256U };

	if (!l_isLoadSuccess)
	{
		m_spriteDrawRequestData->m_scale = { 256.0F, 256.0F };
	}

		  auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderGraph     = l_graphicsManager.GetREFRenderer      ().GetREFRenderGraph();

	const auto& l_spriteScreenPerObjectDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::SpriteScreenPerObjectDrawRequest>().lock();

	if (!l_spriteScreenPerObjectDrawRequest) { return; }

	l_spriteScreenPerObjectDrawRequest->AddDrawRequestPerObject(m_spriteDrawRequestData);

	// モデル
	m_staticModel->Load("Asset/Model/Antike.fbx");
}

void FWK::Scene::Update()
{
	if (GetAsyncKeyState('B'))
	{
		m_texture			    = nullptr;
		m_spriteDrawRequestData = nullptr;
	}

	if (GetAsyncKeyState('A'))
	{
		m_staticModel = nullptr;
	}
}