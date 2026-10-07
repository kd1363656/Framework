#pragma once

namespace FWK::Graphics
{
    class RenderGraph;
    class ShadowContext;
}

namespace FWK::Graphics
{
    class CameraContext final
    {
    public:

         CameraContext() = default;
        ~CameraContext() = default;

        void ApplyViewCamera(const RenderGraph& a_renderGraph, ShadowContext& a_shadowContext) const;

        void SetDebugCamera(const std::weak_ptr<Struct::CBCameraPass>& a_set) { m_debugCamera = a_set; }
        void SetSceneCamera(const std::weak_ptr<Struct::CBCameraPass>& a_set) { m_sceneCamera = a_set; }

        void SetViewMode(const Enum::CameraViewMode a_set) { m_viewMode = a_set; }

    private:

        std::weak_ptr<Struct::CBCameraPass> FetchVALViewCamera() const;

        std::weak_ptr<Struct::CBCameraPass> m_debugCamera = {};
        std::weak_ptr<Struct::CBCameraPass> m_sceneCamera = {};

        Enum::CameraViewMode m_viewMode = Enum::CameraViewMode::Scene;
    };
}