#pragma once

namespace FWK::Concept 
{
	template <typename Type>
	concept IsDerivedConstantBufferBaseConcept = IsDerivedBaseConcept<Type, Graphics::ConstantBufferUploaderBase>;
}