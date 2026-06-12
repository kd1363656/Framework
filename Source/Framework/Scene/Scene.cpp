#include "Scene.h"

void FWK::Scene::INIT()
{
	if (!m_textureOne)
	{
		m_textureOne = std::make_shared<Graphics::Texture>();
	}

	if (!m_textureTwo)
	{
		m_textureTwo = std::make_shared<Graphics::Texture>();
	}

	if (!m_spriteDrawRequestDataOne)
	{
		m_spriteDrawRequestDataOne = std::make_shared<Struct::SpriteScreenPerObjectDrawRequestData>();
	}

	if (!m_spriteDrawRequestDataTwo)
	{
		m_spriteDrawRequestDataTwo = std::make_shared<Struct::SpriteScreenPerObjectDrawRequestData>();
	}

	m_textureOne->Load("Asset/Texture/Test.png",         Enum::TextureLoadType::Color);
	m_textureTwo->Load("Asset/Texture/Body_Texture.png", Enum::TextureLoadType::Color);

	m_spriteDrawRequestDataOne->m_textureRecord = m_textureOne->GetREFTextureRecord();
	m_spriteDrawRequestDataTwo->m_textureRecord = m_textureTwo->GetREFTextureRecord();
	
	m_spriteDrawRequestDataTwo->m_scale = { 0.1F,  0.1F };
	
	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderGraph     = l_graphicsManager.GetREFRenderer      ().GetREFRenderGraph();

	const auto& l_spriteScreenPerObjectDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::SpriteScreenPerObjectDrawRequest>().lock();

	if (!l_spriteScreenPerObjectDrawRequest) { return; }

	l_spriteScreenPerObjectDrawRequest->AddDrawRequestPerObject(m_spriteDrawRequestDataOne);
	l_spriteScreenPerObjectDrawRequest->AddDrawRequestPerObject(m_spriteDrawRequestDataTwo);
}

void FWK::Scene::Update()
{
	if (GetAsyncKeyState('B'))
	{
		m_textureOne			   = nullptr;
		m_spriteDrawRequestDataOne = nullptr;
	}

	if (GetAsyncKeyState('A'))
	{
		m_textureTwo               = nullptr;
		m_spriteDrawRequestDataTwo = nullptr;
	}
}