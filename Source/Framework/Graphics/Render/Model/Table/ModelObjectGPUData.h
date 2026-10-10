#pragma once

namespace FWK::Graphics
{
    class ModelObjectGPUData final
    {
    public:

         ModelObjectGPUData() = default;
        ~ModelObjectGPUData() = default;

        void SetWorldMatrix                (const TypeAlias::Math::Matrix& a_set) { m_worldMatrix                 = a_set; }
        void SetWorldInverseTransposeMatrix(const TypeAlias::Math::Matrix& a_set) { m_worldInverseTransposeMatrix = a_set; }

        void SetWorldMAXScale       (const float a_set) { m_worldMAXScale        = a_set; }
        void SetWorldOrientationSign(const float a_set) { m_worldOrientationSign = a_set; }

    private:

        static constexpr float k_initialWorldMAXScale = 1.0F;

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        float m_worldMAXScale        = k_initialWorldMAXScale;
        float m_worldOrientationSign = Constant::k_normalModelWorldOrientationSign;

        FWK_DEFINE_MODEL_RENDER_TABLE_INFO(ModelObjectGPUData)
    };
}