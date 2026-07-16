#pragma once

namespace FWK::Concept 
{
	template <typename Type>
	concept IsDerivedComputeRequestPerObjectBaseConcept = IsDerivedBaseConcept<Type, Graphics::ComputeRequestPerObjectBase>;
}