#pragma once

namespace FWK::Graphics
{
    // RitchToonMaterialを使うSkeletalAnimationModel(従来の方式 = ComputeShaderでスキニング)のMeshの描画申請
    // 表面と輪郭線の両方の描画で使う(同じMeshを、PipelineStateだけ変えて2回描く)
    class SkeletalAnimationModelRitchToonPerObjectDrawRequest final : public SkeletalAnimationModelPerObjectDrawRequestBase
    {
    public:

         SkeletalAnimationModelRitchToonPerObjectDrawRequest()          = default;
        ~SkeletalAnimationModelRitchToonPerObjectDrawRequest() override = default;

        TypeAlias::StaticTypeID GetVALTargetMaterialStaticTypeID() const override { return RitchToonMaterial::GetREFTypeINFO().k_staticTypeID; }

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelRitchToonPerObjectDrawRequest, SkeletalAnimationModelPerObjectDrawRequestBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationModelRitchToonPerObjectDrawRequest)