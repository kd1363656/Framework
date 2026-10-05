#pragma once

namespace FWK::Editor
{
    class EditorCamera;
}

namespace FWK::Converter
{
    class EditorCameraJsonConverter final
    {
    public:

         EditorCameraJsonConverter() = default;
        ~EditorCameraJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Editor::EditorCamera& a_editorCamera) const;

        nlohmann::json Serialize(const Editor::EditorCamera& a_editorCamera) const;

    private:

        static constexpr std::string_view k_scaleJsonKey       = "Scale";
        static constexpr std::string_view k_rotationJsonKey    = "Rotation";
        static constexpr std::string_view k_positionJsonKey    = "Position";
        static constexpr std::string_view k_fovYDegreeJsonKey  = "FovYDegree";
        static constexpr std::string_view k_farClipJsonKey     = "FarClip";
        static constexpr std::string_view k_nearClipJsonKey    = "NearClip";
        static constexpr std::string_view k_moveSpeedJsonKey   = "MoveSpeed";
        static constexpr std::string_view k_rotateSpeedJsonKey = "RotateSpeed";
    };
}