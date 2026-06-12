#pragma once

namespace FWK::Concept
{
	template <typename Type>
	concept IsDerivedRenderGraphPassBaseConcept = IsDerivedBaseConcept<Type, Graphics::RenderGraphPassBase>;
}