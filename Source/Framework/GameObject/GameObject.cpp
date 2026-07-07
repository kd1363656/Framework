#include "GameObject.h"

void FWK::GameObject::INIT()
{
	if (!m_transformComponent)
	{
		m_transformComponent = std::make_shared<TransformComponent>();
	}
}