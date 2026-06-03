#pragma once

namespace FWK::Utility
{
	template <Concept::IsDerivedTagBaseConcept Type>
	inline auto GetVALTag()
	{
		return Type::GetREFTypeINFO().k_staticTypeID;
	}
}