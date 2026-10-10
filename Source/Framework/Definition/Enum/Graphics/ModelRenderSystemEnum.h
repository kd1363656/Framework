#pragma once

namespace FWK::Enum
{
    enum class ModelRenderTableType
    {
        Invalid,
        Object,
        Mesh,
        StandardLitMaterial,
        StandardUnLitMaterial,
        Count,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        ModelRenderTableType,
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::Invalid),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::Object),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::Mesh),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::StandardLitMaterial),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::StandardUnLitMaterial),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::Count),
    )
}