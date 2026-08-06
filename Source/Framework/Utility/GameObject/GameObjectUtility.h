#pragma once

namespace FWK::Utility
{
	inline bool IsValidPrefabHierarchy(const GameObject& a_rootGameObject)
	{
		return true;
	}

	inline bool IsValidPrefabHierarchy(const nlohmann::json& a_rootJson)
	{
		return true;
	}

	inline bool CanBuildPrefabHierarchy(const GameObject& a_parentGameObject, const GameObject& a_childGameObject)
	{
		return true;
	}
}