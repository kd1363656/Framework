#pragma once

namespace FWK::Struct
{
    // エディターで編集する、モーフターゲット1つ分の設定(.morphファイルに保存する)
    // ターゲットは名前で指す(同じ名前のターゲットを持つ全Meshに同じ設定を使う)
    struct ModelMorphTargetSetting final
    {
        std::wstring m_targetName = {};

        Enum::ModelMorphLODProtection m_lodProtection = Enum::ModelMorphLODProtection::Normal;

        float m_defaultWeight = Constant::k_defaultModelMorphTargetWeight;
        float m_minWeight     = Constant::k_defaultModelMorphTargetMINWeight;
        float m_maxWeight     = Constant::k_defaultModelMorphTargetMAXWeight;

        bool m_isShadowEnabled = true;
    };
}