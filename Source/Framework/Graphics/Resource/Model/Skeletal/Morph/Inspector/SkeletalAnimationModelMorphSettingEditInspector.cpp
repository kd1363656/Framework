#include "SkeletalAnimationModelMorphSettingEditInspector.h"

bool FWK::Graphics::SkeletalAnimationModelMorphSettingEditInspector::EditInspector(SkeletalAnimationModelMorphSetting& a_morphSetting) const
{
    auto& l_targetSettingList = a_morphSetting.GetMutableREFTargetSettingList();

    if (l_targetSettingList.empty())
    {
        ImGui::TextUnformatted(k_emptyTargetText.data());

        return false;
    }

    ImGui::TextUnformatted(k_lodNoticeText.data());

    bool l_isChanged = false;

    for (std::size_t l_targetIndex = 0ULL; l_targetIndex < l_targetSettingList.size(); ++l_targetIndex)
    {
        auto& l_targetSetting = l_targetSettingList[l_targetIndex];

        // 同じラベルの部品が複数並んでもImGuiが区別できるように、ターゲットの番号をIDとして積む
        ImGui::PushID(static_cast<int>(l_targetIndex));

        const auto& l_targetName = Utility::WStringToString(l_targetSetting.m_targetName);

        if (ImGui::TreeNode(l_targetName.c_str()))
        {
            l_isChanged = EditTargetSetting(l_targetSetting) || l_isChanged;

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    return l_isChanged;
}

bool FWK::Graphics::SkeletalAnimationModelMorphSettingEditInspector::EditTargetSetting(Struct::ModelMorphTargetSetting& a_targetSetting) const
{
    bool l_isChanged = false;

    // LOD保護の変更
    // ImGui::Comboは選んだ番号をintで受け取るため、Enumを番号に変えてから渡す
    if (int l_lodProtectionIndex = static_cast<int>(a_targetSetting.m_lodProtection);
        ImGui::Combo(k_lodProtectionLabel.data(), &l_lodProtectionIndex, k_lodProtectionNameList.data(), static_cast<int>(k_lodProtectionNameList.size())))
    {
        a_targetSetting.m_lodProtection = static_cast<Enum::ModelMorphLODProtection>(l_lodProtectionIndex);

        l_isChanged = true;
    }

    // ウェイトの範囲の変更(最小 <= 最大になるように、ImGuiが2つの値をまとめて扱う)
    if (ImGui::DragFloatRange2(k_weightRangeLabel.data(),
                               &a_targetSetting.m_minWeight,
                               &a_targetSetting.m_maxWeight,
                               k_weightRangeDragSpeed,
                               k_weightRangeMIN,
                               k_weightRangeMAX,
                               k_weightRangeMINLabel.data(),
                               k_weightRangeMAXLabel.data()))
    {
        a_targetSetting.m_defaultWeight = std::clamp(a_targetSetting.m_defaultWeight, a_targetSetting.m_minWeight, a_targetSetting.m_maxWeight);

        l_isChanged = true;
    }

    // 既定のウェイトの変更(ウェイトの範囲の中だけで動かせる)
    if (ImGui::SliderFloat(k_defaultWeightLabel.data(), &a_targetSetting.m_defaultWeight, a_targetSetting.m_minWeight, a_targetSetting.m_maxWeight))
    {
        l_isChanged = true;
    }

    // 影でもモーフするかの変更
    if (ImGui::Checkbox(k_isShadowLabel.data(), &a_targetSetting.m_isShadowEnabled))
    {
        l_isChanged = true;
    }

    return l_isChanged;
}