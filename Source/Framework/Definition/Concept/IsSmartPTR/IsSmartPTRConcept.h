#pragma once

namespace FWK::Concept
{
	// シェアードポインタ、ユニークポインタといったスマートポインタかどうかを判別するコンセプト
	template<typename Type>
	concept IsSharedPTRConcept = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Shared;

	template<typename Type>
	concept IsUniquePTRConcept = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Unique;

	template <typename Type>
	concept IsSmartPTRConcept = IsSharedPTRConcept<Type> || 
								IsUniquePTRConcept<Type>;
}