#pragma once

namespace FWK::Concept
{
    template <typename Type>
    concept IsModelRenderTableElementConcept = requires
    {
        { Type::GetREFModelRenderTableINFO() } -> std::same_as<const Struct::ModelRenderTableINFO&>;
    };

    template <typename Type>
    concept IsModelMaterialRenderTableElementConcept = IsModelRenderTableElementConcept<Type> &&
                                                       Type::k_isModelMaterialRenderTableElement;
}