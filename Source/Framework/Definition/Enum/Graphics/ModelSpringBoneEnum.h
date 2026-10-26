#pragma once

namespace FWK::Enum
{
    // 揺れもの(SpringBone)が貫通しないように押し出す、当たり判定の形
    // ※ .springファイルに数値のまま保存するため、並び順を変えない
    enum class ModelSpringBoneColliderShape : std::uint32_t
    {
        // 球(中心と半径)
        Sphere,

        // カプセル(2点を結ぶ線分と半径、腕や脚のような細長い部分に使う)
        Capsule,

        Count,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        ModelSpringBoneColliderShape,
        FWK_JSON_ENUM_VALUE(ModelSpringBoneColliderShape::Sphere),
        FWK_JSON_ENUM_VALUE(ModelSpringBoneColliderShape::Capsule),
        FWK_JSON_ENUM_VALUE(ModelSpringBoneColliderShape::Count)
    )
}