#pragma once

namespace FWK::Graphics
{
    class ModelMaterialBase;
}

namespace FWK::Concept
{
    template <typename Type>
    concept IsDerivedModelMaterialBaseConcept = IsDerivedBaseConcept<Type, Graphics::ModelMaterialBase>;
}