#pragma once

namespace FWK::Utility
{
    template <typename Type>
        requires std::is_enum_v<Type>
    class EnumBitShift;
}

namespace FWK::Converter
{
    // Enum値の配列としてデシリアライズ、シリアライズを行う
    // 呼び出し側がキーを指定して配列Jsonを受け渡しする設計
    // EnumはFWK_JSON_SERIALIZE_ENUMにより文字列へ変換されている前提
    template <typename Type>
        requires std::is_enum_v<Type>
    class EnumBitShiftJsonConverter final
    {
    public:

         EnumBitShiftJsonConverter() = default;
        ~EnumBitShiftJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Utility::EnumBitShift<Type>& a_enumBitShift) const
        {
            if (a_rootJson.is_null() ||
                !Utility::IsJsonArray(a_rootJson))
            {
                return;
            }

            // 既存フラグを残すとDeserialize前の状態が混ざるためクリアする
            auto& l_bitShiftFlagList = a_enumBitShift.GetMutableREFBitShiftFlagList();

            l_bitShiftFlagList.clear();

            a_enumBitShift.SetBitShiftFlag(static_cast<std::uint32_t>(Type::Invalid));

            for (const auto& l_json : a_rootJson)
            {
                if (l_json.is_null()) { continue; }

                // AddBitShiftFlag内で重複チェックとビット値の同期が行われる
                const auto l_bitShiftFlag = a_rootJson.value(k_bitShiftFlagJsonKey, Type::Invalid);

                a_enumBitShift.AddBitShiftFlag(l_bitShiftFlag);
            }
        }

        nlohmann::json Serialize(const Utility::EnumBitShift<Type>& a_enumBitShift) const
        {
            auto l_jsonArray = nlohmann::json::array();

            for (const auto& l_flag : a_enumBitShift.GetREFBitShiftFlagList())
            {
                nlohmann::json l_json = {};

                l_json[k_bitShiftFlagJsonKey] = l_flag;

                l_jsonArray.emplace_back(l_json);
            }

            return l_jsonArray;
        }

    private:

        static constexpr std::string_view k_bitShiftFlagJsonKey = "BitShiftFlag";
    };
}
