#pragma once

namespace FWK::Utility
{
	inline bool IsDerivedFrom(const TypeINFORegistry::TypeINFO& a_typeINFO, const TypeINFORegistry::TypeINFO& a_baseTypeINFO)
	{
		const auto* l_currentINFO = &a_typeINFO;

		while (l_currentINFO)
		{
			if (l_currentINFO->k_staticTypeID == a_baseTypeINFO.k_staticTypeID)
			{
				return true;
			}

			l_currentINFO = l_currentINFO->k_baseINFO;
		}

		return false;
	}
}