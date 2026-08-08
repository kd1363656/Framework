#pragma once

namespace FWK::Struct
{
	struct GameObjectNodeHierarchyChangeRequest final
	{
		std::weak_ptr<GameObject> m_parentGameObject = {};
		std::weak_ptr<GameObject> m_childGameObject  = {};

		Enum::GameObjectHierarchyChangeRequestType m_type = Enum::GameObjectHierarchyChangeRequestType::None;
	};

	struct GameObjectNodeDrawResult final
	{
		bool m_hasChildGameObject = false;
		bool m_isNodeOpen         = false;
	};
}