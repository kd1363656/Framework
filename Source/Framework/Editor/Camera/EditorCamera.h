#pragma once

namespace FWK::Editor
{
    class EditorCamera final
    {
    public:

         EditorCamera() = default;
        ~EditorCamera() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        void Setup(const float a_aspectRatio);

        void HandleInput(const bool a_isHovered);

        nlohmann::json Serialize() const;

        void ApplyTransformPosition(const TypeAlias::Math::Vector3&    a_set);
        void ApplyTransformRotation(const TypeAlias::Math::Quaternion& a_set);
        void ApplyTransformScale   (const TypeAlias::Math::Vector3&    a_set);

        void ApplyAspectRatio(const float a_set);
        void ApplyFovYDegree (const float a_set);

        void SetMoveSpeed  (const float a_set) { m_moveSpeed   = a_set; }
        void SetRotateSpeed(const float a_set) { m_rotateSpeed = a_set; }

        const auto& GetREFCamera() const { return m_camera;    }
        
        const auto& GetREFTransform() const { return m_transform; }

        auto& GetMutableREFCamera() { return m_camera; }
        
        float GetVALMoveSpeed  () const { return m_moveSpeed;   }
        float GetVALRotateSpeed() const { return m_rotateSpeed; }

    private:

        void Move  (const TypeAlias::Math::Vector3& a_localDirection, const float a_deltaTime);
        void Rotate(const float                     a_yawRadian,      const float a_pitchRadian);

        TypeAlias::Math::Matrix CalculateCameraMatrix() const;

        void UpdateCameraTransform();

        Graphics::Camera m_camera = {};

        Converter::EditorCameraJsonConverter m_jsonConverter = {};
        
        Struct::Transform m_transform = {};
        
        float m_moveSpeed   = Constant::k_editorCameraDefaultMoveSpeed;
        float m_rotateSpeed = Constant::k_editorCameraDefaultRotateSpeed;
    };
}