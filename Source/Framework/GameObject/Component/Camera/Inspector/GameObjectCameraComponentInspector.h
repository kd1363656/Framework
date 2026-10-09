#pragma once

namespace FWK
{
    class GameObjectCameraComponent;
}

namespace FWK
{
    class GameObjectCameraComponentInspector final
    {
    public:

         GameObjectCameraComponentInspector() = default;
        ~GameObjectCameraComponentInspector() = default;

        void EditInspector(GameObjectCameraComponent& a_cameraComponent) const;

    private:

        static constexpr std::string_view k_aspectRatioLabel = "アスペクト比 : %.2f";
        static constexpr std::string_view k_fovYDegreeLabel  = "視野角";
        static constexpr std::string_view k_farClipLabel     = "ファークリップ";
        static constexpr std::string_view k_nearClipLabel    = "ニアークリップ";
    };
}