#include "Scene.h"

void FWK::Scene::INIT()
{
	auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance   ();
	auto& l_resourceContext = l_graphicsManager.GetMutableREFResourceContext();
	auto& l_textureSystem   = l_resourceContext.GetMutableREFTextureSystem  ();

	l_textureSystem.LoadTextureFile("Asset/Texture/Test.png", Enum::TextureLoadType::Color);
}