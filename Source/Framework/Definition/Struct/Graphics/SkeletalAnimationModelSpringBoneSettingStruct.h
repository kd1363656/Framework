#pragma once

namespace FWK::Struct
{
    // エディターで編集する、揺れもの(SpringBone)の「チェーン」1グループ分の設定(.springファイルに保存する)
    //
    // チェーン : 根元のボーンから「最初の子ボーン」を順にたどった、数珠つなぎのボーンの列
    //           例 : Skirt1.Front.L.000 → Skirt1.Front.L.001 → … → Skirt1.Front.L.006
    // グループ : 同じパラメーターで揺らすチェーンの集まり(スカートの前後左右のチェーンなど)
    //
    // m_rootBoneNameList の並び順 : 隣同士をつなぐ(m_isAdjacentChainLinked)ときの「隣」の順番になる
    //                               スカートなら、腰の周りを一周する順に並べる
    struct ModelSpringBoneChainSetting final
    {
        std::vector<std::wstring> m_rootBoneNameList = {};

        TypeAlias::Math::Vector3 m_gravityDirection = Constant::k_defaultModelSpringBoneGravityDirection;

        float m_stiffness                = Constant::k_defaultModelSpringBoneStiffness;
        float m_drag                     = Constant::k_defaultModelSpringBoneDrag;
        float m_gravityPower             = Constant::k_defaultModelSpringBoneGravityPower;
        float m_hitRadius                = Constant::k_defaultModelSpringBoneHitRadius;
        float m_maxAngularVelocityDegree = Constant::k_defaultModelSpringBoneMAXAngularVelocityDegree;
        float m_endLength                = Constant::k_defaultModelSpringBoneEndLength;
        float m_adjacentLinkStiffness    = Constant::k_defaultModelSpringBoneAdjacentLinkStiffness;

        bool m_isEnabled             = true;
        bool m_isAdjacentChainLinked = false;
        bool m_isAdjacentChainLooped = false;
    };

    // エディターで編集する、揺れものが貫通しないように押し出す当たり判定1つ分の設定(.springファイルに保存する)
    // m_offset、m_tailOffset はボーンの座標系での位置(m_tailOffset はカプセルのときだけ使う)
    struct ModelSpringBoneColliderSetting final
    {
        std::wstring m_boneName = {};

        TypeAlias::Math::Vector3 m_offset     = Constant::k_defaultModelSpringBoneColliderOffset;
        TypeAlias::Math::Vector3 m_tailOffset = Constant::k_defaultModelSpringBoneColliderTailOffset;

        Enum::ModelSpringBoneColliderShape m_shape = Enum::ModelSpringBoneColliderShape::Sphere;

        float m_radius = Constant::k_defaultModelSpringBoneColliderRadius;
    };
}