#pragma once

namespace FWK::Graphics
{
    // GPUスキニングで描画するSkeletalAnimationModelの、通常描画(ライティングあり)の描画申請
    class SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequest final : public SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase
    {
    public:

         SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequest()          = default;
        ~SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequest() override = default;

        TypeAlias::StaticTypeID GetVALTargetMaterialStaticTypeID() const override { return StandardLitMaterial::GetREFTypeINFO().k_staticTypeID; }

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequest, SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequest)