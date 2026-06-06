#pragma once

namespace FWK::Utility::Tag
{
	template <Concept::IsDerivedTagBaseConcept Type>
	inline auto GetVALTag() 
	{
		return Type::GetREFTypeINFO().k_staticTypeID; 
	}
}