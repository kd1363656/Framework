#pragma once

namespace FWK::Graphics
{
    class StaticModelCascadeShadowPerObjectDrawRequest final : public DrawRequestPerObjectBase
    {
    public:

         StaticModelCascadeShadowPerObjectDrawRequest()          = default;
        ~StaticModelCascadeShadowPerObjectDrawRequest() override = default;

        void BeginFrame() override;

        void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

        void AddDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData);

        // カスケード1枚ごとに、影を落とすMeshの選別とLOD選択をGPUで行う(View番号 = カスケード番号)
        bool DispatchGPUCulling(Renderer& a_renderer, const std::size_t a_cascadeIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView);

        void SetCurrentDrawCascadeIndex(const std::size_t a_set) { m_staticModelPerObjectDrawRequest.SetCurrentDrawViewIndex(a_set); }

    private:

        StaticModelPerObjectDrawRequestBase m_staticModelPerObjectDrawRequest = {};

        FWK_DEFINE_TYPE_INFO(StaticModelCascadeShadowPerObjectDrawRequest, DrawRequestPerObjectBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::StaticModelCascadeShadowPerObjectDrawRequest)