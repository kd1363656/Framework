#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK::Struct
{
	struct ChildDeserializeData final
	{
		std::vector<ChildDeserializeData> m_childDeserializeDataList = {};

		std::shared_ptr<GameObject> m_self = nullptr;

		Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>> m_componentSmartPointerVectorArray = {};
	};
}