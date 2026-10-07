#pragma once

namespace FWK::Editor
{
    class EditorDebugRendererQueue final
    {
    public:

         EditorDebugRendererQueue() = default;
        ~EditorDebugRendererQueue() = default;

        void ClearFrame();

        void AddLine(const TypeAlias::Math::Vector3& a_startPosition, const TypeAlias::Math::Vector3& a_endPosition, const TypeAlias::Math::Color& a_color);

        void AddFrustum(const TypeAlias::Math::Matrix& a_worldMatrix,
                        const TypeAlias::Math::Color&  a_color,
                        const float                    a_tanHalfFOVX,
                        const float                    a_tanHalfFOVY,
                        const float                    a_nearDistance,
                        const float                    a_farDistance);

        bool HasLineVertex() const { return !m_lineVertexList.empty(); }

        const auto& GetREFLineVertexList() const { return m_lineVertexList; }

    private:

        static constexpr std::size_t k_planeCornerCount = 4ULL;
        static constexpr std::size_t k_nextCornerOffset = 1ULL;

        std::vector<Struct::VBEditorCameraDebug> m_lineVertexList = {};
    };
}