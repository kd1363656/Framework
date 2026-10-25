#pragma once

namespace FWK::Enum
{
    // LOD(遠距離用に三角形を減らした形)を作るときに、モーフターゲットの形をどこまで守るか
    // ※ .morphファイルとモデルのバイナリーファイルに数値のまま保存するため、並び順を変えない
    enum class ModelMorphLODProtection : std::uint32_t
    {
        // 他の頂点属性と同じように、形の崩れを誤差として扱う
        Normal,

        // 形の崩れを大きく見積もり、遠くなるまで崩れるLODを使わない
        Important,

        // このターゲットで動く頂点は、LODでも動かさない(消さない)
        Lock,

        Count,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        ModelMorphLODProtection,
        FWK_JSON_ENUM_VALUE(ModelMorphLODProtection::Normal),
        FWK_JSON_ENUM_VALUE(ModelMorphLODProtection::Important),
        FWK_JSON_ENUM_VALUE(ModelMorphLODProtection::Lock),
        FWK_JSON_ENUM_VALUE(ModelMorphLODProtection::Count)
    )
}