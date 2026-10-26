#pragma once

namespace FWK::Graphics
{
    // GPUスキニングで描画するSkeletalAnimationModelの、影(カスケードシャドウ)の描画申請
    class SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest final : public DrawRequestPerObjectBase
    {
    public:

         SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest()          = default;
        ~SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest() override = default;

        void BeginFrame() override;

        void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

        void AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelGPUSkinnedDrawRequestData>& a_drawRequestData);

        // カスケード1枚ごとに、影を落とすMeshの選別とLOD選択をGPUで行う(View番号 = カスケード番号)
        bool DispatchGPUCulling(Renderer& a_renderer, const std::size_t a_cascadeIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView);

        void SetCurrentDrawCascadeIndex(const std::size_t a_set) { m_skeletalAnimationModelGPUSkinnedPerObjectDrawRequest.SetCurrentDrawViewIndex(a_set); }

    private:

        SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase m_skeletalAnimationModelGPUSkinnedPerObjectDrawRequest = {};

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest, DrawRequestPerObjectBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest)