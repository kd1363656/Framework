#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK::Struct
{
	struct ChildDeserializeData final
	{
		std::vector<std::shared_ptr<ComponentBase>> m_componentList            = {};
		std::vector<ChildDeserializeData>           m_childDeserializeDataList = {};

		std::shared_ptr<GameObject> m_self = nullptr;
	};
}