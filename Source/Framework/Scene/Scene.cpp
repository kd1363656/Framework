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

	m_texture->Load("Asset/Texture/Test.png", Enum::TextureLoadType::Color);

	m_spriteDrawRequestData->m_textureRecord = m_texture->GetREFTextureRecord();
	m_spriteDrawRequestData->m_sourceRECT    = { 0U, 0U, 256U, 256U };

	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderGraph     = l_graphicsManager.GetREFRenderer      ().GetREFRenderGraph();

	const auto& l_spriteScreenPerObjectDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::SpriteScreenPerObjectDrawRequest>().lock();

	if (!l_spriteScreenPerObjectDrawRequest) { return; }

	l_spriteScreenPerObjectDrawRequest->AddDrawRequestPerObject(m_spriteDrawRequestData);

	Graphics::StaticModelFBXLoader l_loader = {};

	const std::filesystem::path& l_filePath = "Asset/Model/Antike.fbx";
	Graphics::StaticModelRecord  l_record   = {};

	l_loader.LoadStaticModelFile(l_filePath, l_record);
}

void FWK::Scene::Update()
{
	if (GetAsyncKeyState('B'))
	{
		m_texture			    = nullptr;
		m_spriteDrawRequestData = nullptr;
	}
}