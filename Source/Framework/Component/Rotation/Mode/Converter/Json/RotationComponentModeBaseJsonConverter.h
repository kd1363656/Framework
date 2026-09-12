#pragma once

namespace FWK
{
    class RotationComponentModeBase;
}

namespace FWK::Converter
{
    class RotationComponentModeBaseJsonConverter final
    {
    public:

         RotationComponentModeBaseJsonConverter() = default;
        ~RotationComponentModeBaseJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_moveComponentModeBase) const;

        nlohmann::json Serialize(const RotationComponentModeBase& a_moveComponentModeBase) const;

    private:

        void DeserializeCanApplyRotationAxisBitShiftFlagList(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_moveComponentModeBase) const;

        nlohmann::json SerializeCanApplyRotationAxisBitShiftFlagList(const RotationComponentModeBase& a_moveComponentModeBase) const;

        static constexpr std::string_view k_canApplyRotationAxisBitShiftFlagListJsonKey = "RotationApplyAxisBitShiftFlagList";
        static constexpr std::string_view k_canApplyRotationAxisJsonKey                 = "RotationApplyAxisBitShiftFlag";
    };
}