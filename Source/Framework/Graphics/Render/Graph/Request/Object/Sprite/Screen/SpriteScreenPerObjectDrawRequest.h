#pragma once

namespace FWK::Graphics
{
    class Renderer;
}

namespace FWK::Graphics
{
    class SpriteScreenPerObjectDrawRequest final : public DrawRequestPerObjectBase
    {
    public:

         SpriteScreenPerObjectDrawRequest()          = default;
        ~SpriteScreenPerObjectDrawRequest() override = default;

        void BeginFrame() override;

        void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

        void AddDrawRequestPerObject(const std::shared_ptr<Struct::SpriteScreenDrawRequestData>& a_drawRequestData);

    private:

        Utility::SmartPointerVectorList<std::weak_ptr<Struct::SpriteScreenDrawRequestData>> m_drawRequestDataSmartPointerVectorList = {};

        FWK_DEFINE_TYPE_INFO(SpriteScreenPerObjectDrawRequest, DrawRequestPerObjectBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SpriteScreenPerObjectDrawRequest)