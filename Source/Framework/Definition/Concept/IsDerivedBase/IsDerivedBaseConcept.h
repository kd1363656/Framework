#pragma once

namespace FWK::Concept
{
	// 継承関係があるかどうかを調べるコンセプト
	template <typename Type, typename BaseType>
	concept IsDerivedBaseConcept = std::derived_from<Type, BaseType>;
}