#pragma once

namespace FWK::Utility
{
	inline std::string_view FetchVALPrefabName(const GameObject& a_gameObject)
	{
		if (a_gameObject.GetVALPrefabInstanceNUM() == Constant::k_invalidPrefabInstanceNUM) { return {}; }

		return a_gameObject.GetREFPrefabName();
	}

	inline bool IsValidPrefabHierarchy(const GameObject& a_gameObject)
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