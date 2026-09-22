#pragma once

namespace FWK::Concept
{
    template <typename Type>
    concept IsDerivedICommandConcept = Concept::IsDerivedBaseConcept<Type, ICommand>;
}