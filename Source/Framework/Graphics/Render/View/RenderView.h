#pragma once

namespace FWK::Graphics
{
    class RenderGraph;
    class ShadowContext;
}

namespace FWK::Graphics
{
    class RenderView final
    {
    public:

         RenderView() = default;
        ~RenderView() = default;

        bool ApplyViewCamera(const RenderGraph& a_renderGraph, ShadowContext& a_shadowContext) const;

        const auto& GetREFRenderArea() const { return m_renderArea; }

        auto& GetMutableREFRenderArea   () { return m_renderArea; }
        auto& GetMutableREFCameraContext() { return m_cameraContext; }

    private:

        RenderArea    m_renderArea    = {};
        CameraContext m_cameraContext = {};
    };
}