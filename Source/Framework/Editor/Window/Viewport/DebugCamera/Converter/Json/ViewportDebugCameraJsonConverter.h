#pragma once

namespace FWK::Editor
{
    class ViewportDebugCamera;
}

namespace FWK::Converter
{
    class ViewportDebugCameraJsonConverter final
    {
    public:

         ViewportDebugCameraJsonConverter() = default;
        ~ViewportDebugCameraJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Editor::ViewportDebugCamera& a_debugCamera) const;

        nlohmann::json Serialize(const Editor::ViewportDebugCamera& a_debugCamera) const;

    private:

        static constexpr std::string_view k_positionJsonKey                = "Position";
        static constexpr std::string_view k_yawDegreeJsonKey               = "YawDegree";
        static constexpr std::string_view k_pitchDegreeJsonKey             = "PitchDegree";
        static constexpr std::string_view k_moveSpeedJsonKey               = "MoveSpeed";
        static constexpr std::string_view k_rotationSpeedDegreeJsonKey     = "RotationSpeedDegree";
        static constexpr std::string_view k_isEnabledJsonKey               = "IsEnabled";
        static constexpr std::string_view k_isCullingDebugVisualizeJsonKey = "IsCullingDebugVisualize";
        static constexpr std::string_view k_isDrawMainCameraFrustumJsonKey = "IsDrawMainCameraFrustum";
    };
}
