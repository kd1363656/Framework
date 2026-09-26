#pragma once

namespace FWK::Utility
{
    template <typename Type>
        requires std::is_enum_v<Type>
    class EnumBitShift;
}

namespace FWK::Utility
{
    // EnumBitShiftのフラグをCheckBoxで編集するInspector
    // TypeのEnumはStringValueBidirectionalRegistryへの登録が必要
    template <typename Type>
        requires std::is_enum_v<Type>
    class EnumBitShiftInspector final
    {
    public:

         EnumBitShiftInspector() = default;
        ~EnumBitShiftInspector() = default;

        // 変更があった場合はtrueを返す
        bool EditInspector(const std::string_view& a_label, EnumBitShift<Type>& a_enumBitShift)
        {
            auto& l_shiftFlagList = a_enumBitShift.GetMutableREFBitShiftFlagList();

            // CheckBoxSelectorがフラグリストへ直接追加・削除を行う
            // 変更がなければビット値を再構築する必要はない
            if (!Utility::IMGUIStringValueBidirectionalRegistryCheckBoxSelector(a_label, l_shiftFlagList))
            {
                return false;
            }

            // ビット値をInvalid値へ戻してからリストの内容で再構築する
            a_enumBitShift.SetBitShiftFlag(static_cast<std::uint32_t>(Type::Invalid));

            for (const auto& l_flag : a_enumBitShift.GetREFBitShiftFlagList())
            {
                auto l_bitShiftFlag = a_enumBitShift.GetVALBitShiftFlag();

                a_enumBitShift.SetBitShiftFlag(Utility::EnableFlag(l_flag, l_bitShiftFlag));
            }

            return true;
        }
    };
}
