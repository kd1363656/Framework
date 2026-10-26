#pragma once

namespace FWK::Graphics
{
    // RitchToonMaterialを使うStaticModelのMeshの描画申請
    // 表面と輪郭線の両方の描画で使う(同じMeshを、PipelineStateだけ変えて2回描く)
    class StaticModelRitchToonPerObjectDrawRequest final : public StaticModelPerObjectDrawRequestBase
    {
    public:

         StaticModelRitchToonPerObjectDrawRequest()          = default;
        ~StaticModelRitchToonPerObjectDrawRequest() override = default;

        TypeAlias::StaticTypeID GetVALTargetMaterialStaticTypeID() const override { return RitchToonMaterial::GetREFTypeINFO().k_staticTypeID; }

        FWK_DEFINE_TYPE_INFO(StaticModelRitchToonPerObjectDrawRequest, StaticModelPerObjectDrawRequestBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::StaticModelRitchToonPerObjectDrawRequest)