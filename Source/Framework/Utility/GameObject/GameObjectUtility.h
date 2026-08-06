#pragma once

namespace FWK::Utility
{
	inline bool IsPrefabInstance(const GameObject& a_gameObject)
	{
		return !a_gameObject.GetREFPrefabName       ().empty() &&
			    a_gameObject.GetVALPrefabInstanceNUM() != Constant::k_invalidPrefabInstanceNUM;
	}
}