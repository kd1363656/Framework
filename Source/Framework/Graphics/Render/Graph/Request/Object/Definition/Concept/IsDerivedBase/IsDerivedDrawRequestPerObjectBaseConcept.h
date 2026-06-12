#pragma once

namespace FWK::Concept
{
	template <typename Type>
	concept IsDerivedDrawRequestPerObjectBaseConcept = IsDerivedBaseConcept<Type, Graphics::DrawRequestPerObjectBase>;
}