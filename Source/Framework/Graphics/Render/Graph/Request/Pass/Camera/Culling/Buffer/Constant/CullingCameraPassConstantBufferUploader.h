#pragma once

namespace FWK::Graphics
{
    class CullingCameraPassDynamicConstantBufferUploader final : public DynamicConstantBufferAdvancingWritePositionUploaderBase<Struct::CBCullingCameraPass>
    {
    public:

         CullingCameraPassDynamicConstantBufferUploader()          = default;
        ~CullingCameraPassDynamicConstantBufferUploader() override = default;

        FWK_DEFINE_TYPE_INFO(CullingCameraPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::CullingCameraPassDynamicConstantBufferUploader)