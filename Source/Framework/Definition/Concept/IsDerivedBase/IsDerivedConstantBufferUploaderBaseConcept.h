#pragma once

namespace FWK::Concept 
{
	template <typename Type>
	concept IsDerivedConstantBufferUploaderBaseConcept = IsDerivedBaseConcept<Type, Graphics::ConstantBufferUploaderBase>;
}