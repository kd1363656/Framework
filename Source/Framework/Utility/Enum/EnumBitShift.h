#pragma once

namespace FWK::Converter
{
    template <typename Type>
        requires std::is_enum_v<Type>
    class EnumBitShiftJsonConverter;

    template <typename Type>
        requires std::is_enum_v<Type>
    class EnumBitShiftInspector;
}

namespace FWK::Utility
{
    template <typename Type>
        requires std::is_enum_v<Type>
    class EnumBitShift final
    {
    public:

         EnumBitShift() = default;
        ~EnumBitShift() = default;

        void Deserialize(const nlohmann::json& a_rootJson)
        {
            if (a_rootJson.is_null()) { return; }

            m_jsonConverter.Deserialize(a_rootJson, *this);
        }

        nlohmann::json Serialize() const
        {
            return m_jsonConverter.Serialize(*this);
        }

        // リスト内に既にあれば追加しない
        // 追加した場合はtrueを返す
        bool AddBitShiftFlag(const Type& a_addFlag)
        {
            // Invalidはフラグとして無効なため追加しない
            if (a_addFlag == Type::Invalid) { return false; }

            if (std::ranges::find(m_bitShiftFlagList, a_addFlag) != m_bitShiftFlagList.end())
            {
                FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "既に同じBitShiftFlagが追加されているため、追加処理をスキップしました。");

                return false;
            }

            m_bitShiftFlagList.emplace_back(a_addFlag);

            // リストとビット値がずれないよう追加時にフラグも立てる
            m_bitShiftFlag = Utility::EnableFlag(a_addFlag, m_bitShiftFlag);

            return true;
        }

        // 指定したフラグが有効かどうか
        bool IsFlagEnabled(const Type& a_checkFlag) const
        {
            return Utility::IsFlagEnabled(a_checkFlag, m_bitShiftFlag);
        }

        // InspectorでフラグをCheckBox編集する
        // 変更があった場合はtrueを返す
        bool EditInspector(const std::string_view& a_label)
        {
            return m_inspector.EditInspector(a_label, *this);
        }

        void SetBitShiftFlag(const std::uint32_t a_set) { m_bitShiftFlag = a_set; }

        const auto& GetREFBitShiftFlagList() const { return m_bitShiftFlagList; }

        auto& GetMutableREFBitShiftFlagList() { return m_bitShiftFlagList; }

        auto GetVALBitShiftFlag() const { return m_bitShiftFlag; }

    private:

        Converter::EnumBitShiftJsonConverter<Type> m_jsonConverter = {};

        EnumBitShiftInspector<Type> m_inspector = {};
    
        std::vector<Type> m_bitShiftFlagList = {};

        std::uint32_t m_bitShiftFlag = static_cast<std::uint32_t>(Type::Invalid);
    };
}