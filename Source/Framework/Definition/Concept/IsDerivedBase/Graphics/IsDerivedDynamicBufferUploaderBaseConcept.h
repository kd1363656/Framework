#pragma once

namespace FWK::Concept 
{
	template <typename Type>
	concept IsDerivedDynamicBufferUploaderBaseConcept = IsDerivedBaseConcept<Type, Graphics::DynamicBufferUploaderBase>;
}