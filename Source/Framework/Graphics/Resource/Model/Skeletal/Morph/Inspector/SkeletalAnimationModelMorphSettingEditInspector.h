#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelMorphSetting;
}

namespace FWK::Graphics
{
    // SkeletalAnimationModelMorphSettingの、ターゲットごとの設定をInspectorで編集するクラス
    class SkeletalAnimationModelMorphSettingEditInspector final
    {
    public:

         SkeletalAnimationModelMorphSettingEditInspector() = default;
        ~SkeletalAnimationModelMorphSettingEditInspector() = default;

        // 値が変わった場合はtrueを返す
        bool EditInspector(SkeletalAnimationModelMorphSetting& a_morphSetting) const;

    private:

        bool EditTargetSetting(Struct::ModelMorphTargetSetting& a_targetSetting) const;

        static constexpr std::array<const char*, static_cast<std::size_t>(Enum::ModelMorphLODProtection::Count)> k_lodProtectionNameList = { "通常", "重視", "固定(LODで動かさない)" };

        static constexpr std::string_view k_emptyTargetText     = "モーフターゲットがありません";
        static constexpr std::string_view k_lodProtectionLabel  = "LOD保護";
        static constexpr std::string_view k_defaultWeightLabel  = "既定のウェイト";
        static constexpr std::string_view k_weightRangeMINLabel = "最小";
        static constexpr std::string_view k_weightRangeMAXLabel = "最大";
        static constexpr std::string_view k_weightRangeLabel    = "ウェイトの範囲";
        static constexpr std::string_view k_isShadowLabel       = "影でもモーフする";
        static constexpr std::string_view k_lodNoticeText       = "LOD保護の変更は、保存した後の次回の読み込みで反映されます";

        static constexpr float k_weightRangeDragSpeed = 0.01F;
        static constexpr float k_weightRangeMIN       = -1.0F;
        static constexpr float k_weightRangeMAX       =  2.0F;
    };
}