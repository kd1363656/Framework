#pragma once

namespace FWK::Graphics
{
    class CullingCameraPassDrawRequest final : public DrawRequestPassBase
    {
    public:

         CullingCameraPassDrawRequest()          = default;
        ~CullingCameraPassDrawRequest() override = default;

        bool SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource) override;

        void SetSourceCamera(const std::weak_ptr<Struct::CBCameraPass>& a_set) { m_sourceCamera = a_set; }

        FWK_DEFINE_TYPE_INFO(CullingCameraPassDrawRequest, DrawRequestPassBase)

    private:

        void UpdateConstantBuffer();

        std::weak_ptr<Struct::CBCameraPass> m_sourceCamera = {};

        Struct::CBCullingCameraPass m_constantBuffer = {};
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPassSharedFactory , FWK::Graphics::CullingCameraPassDrawRequest)