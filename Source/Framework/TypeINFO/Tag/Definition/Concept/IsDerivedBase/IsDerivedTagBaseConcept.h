#pragma once

namespace FWK::Concept
{
	// TagBaseを継承しているかどうかを確認するコンセプト
	template <typename Type>
	concept IsDerivedTagBaseConcept = IsDerivedBaseConcept<Type, Tag::TagBase>;
}