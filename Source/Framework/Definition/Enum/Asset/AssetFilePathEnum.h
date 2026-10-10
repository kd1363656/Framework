#pragma once

namespace FWK::Enum
{
    enum class AssetFilePathType
    {
        Invalid,
        Prefab,
        Scene,
        Texture,
        ModelMaterial,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        AssetFilePathType,
        FWK_JSON_ENUM_VALUE(AssetFilePathType::Invalid),
        FWK_JSON_ENUM_VALUE(AssetFilePathType::Prefab),
        FWK_JSON_ENUM_VALUE(AssetFilePathType::Scene),
        FWK_JSON_ENUM_VALUE(AssetFilePathType::Texture),
        FWK_JSON_ENUM_VALUE(AssetFilePathType::ModelMaterial),
    )
}