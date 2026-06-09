#include "Scene.h"

void FWK::Scene::INIT()
{
	if (!m_texture)
	{
		m_texture = std::make_shared<Graphics::Texture>();
	}

	m_texture->Load("Asset/Texture/Test.png", Enum::TextureLoadType::Color);
}

void FWK::Scene::Update()
{
	if (GetAsyncKeyState('B'))
	{
		m_texture = nullptr;
	}
}