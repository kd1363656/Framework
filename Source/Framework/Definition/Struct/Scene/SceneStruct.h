#pragma once

namespace FWK::Struct
{
	struct GameObjectData
	{
		std::shared_ptr<GameObject> m_gameObject = nullptr;

		const GameObject* m_gameObjectAddress = nullptr;
	};
}