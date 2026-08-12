#pragma once

namespace FWK::Utility
{
	inline bool IsPrefabInstance(const GameObject& a_gameObject)
	{
		// PrefabUUIDを持っており、PrefabInstanceNUMも発行済みならPrefabInstanceとして扱う
		return !a_gameObject.GetREFPrefabUUID       ().is_nil() &&
			    a_gameObject.GetVALPrefabInstanceNUM() != Constant::k_invalidPrefabInstanceNUM;
	}

	inline bool IsSamePrefab(const GameObject& a_lhs, const GameObject& a_rhs)
	{
		const auto& l_lhsPrefabUUID = a_lhs.GetREFPrefabUUID();
		const auto& l_rhsPrefabUUID = a_rhs.GetREFPrefabUUID();

		// NilUUID同士は
		// 「同じPrefab」ではなく「どちらもPrefabではない状態」
		if (l_lhsPrefabUUID.is_nil() ||
			l_rhsPrefabUUID.is_nil())
		{
			return false;
		}

		return l_lhsPrefabUUID == l_rhsPrefabUUID;
	}
}