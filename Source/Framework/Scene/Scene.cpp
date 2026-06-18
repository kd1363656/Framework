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

	// テクスチャ
	const bool l_isLoadSuccess = m_texture->Load("Asset/Texture/Te.png", Enum::TextureLoadType::Color);

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
	const auto& l_device = l_graphicsManager.GetREFDevice();

		  auto& l_resourceContext    = l_graphicsManager.GetMutableREFResourceContext  ();
	const auto& l_gpuMemoryAllocator = l_resourceContext.GetREFGPUMemoryAllocator      ();
		  auto& l_staticModelLoader  = l_resourceContext.GetMutableREFStaticModelSystem();
	      auto& l_srvDescriptorPool  = l_resourceContext.GetMutableREFSRVDescriptorPool();

	const std::filesystem::path& l_filePath = "Asset/Model/Antike.fbx";
	
	l_staticModelLoader.LoadStaticModelForBatchUpload(l_device, 
												      l_gpuMemoryAllocator,
													  l_filePath, 
													  l_srvDescriptorPool);
}

void FWK::Scene::Update()
{
	if (GetAsyncKeyState('B'))
	{
		m_texture			    = nullptr;
		m_spriteDrawRequestData = nullptr;
	}
}