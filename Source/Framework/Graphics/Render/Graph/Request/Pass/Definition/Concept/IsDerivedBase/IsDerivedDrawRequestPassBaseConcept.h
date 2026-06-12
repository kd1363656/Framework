#pragma once

namespace FWK::Concept
{
	template <typename Type>
	concept IsDerivedDrawRequestPassBaseConcept = IsDerivedBaseConcept<Type, Graphics::DrawRequestPassBase>;
}