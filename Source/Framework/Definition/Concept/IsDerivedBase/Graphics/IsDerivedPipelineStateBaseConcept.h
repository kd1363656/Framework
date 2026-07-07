#pragma once

namespace FWK::Concept
{
	// PipelineStateBaseを継承したかどうかを確認するConcept
	template <typename Type>
	concept IsDerivedPipelineStateBaseConcept = IsDerivedBaseConcept<Type, Graphics::PipelineStateBase>;
}