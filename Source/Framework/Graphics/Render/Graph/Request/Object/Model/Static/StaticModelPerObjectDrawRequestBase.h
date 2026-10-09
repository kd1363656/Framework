#pragma once

namespace FWK::Graphics
{
    class StaticModelPerObjectDrawRequestBase : public DrawRequestPerObjectBase
    {
    public:

         StaticModelPerObjectDrawRequestBase()          = default;
        ~StaticModelPerObjectDrawRequestBase() override = default;

        void BeginFrame() override;

        void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

        void AddDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData);

    private:

        bool DispatchModelMesh(const DirectCommandList& a_directCommandList, const Struct::StaticModelMesh& a_modelMesh) const;

        static constexpr UINT k_defaultDispatchMeshThreadGroupCountY = 1U;
        static constexpr UINT k_defaultDispatchMeshThreadGroupCountZ = 1U;

        Utility::SmartPointerVectorList<std::weak_ptr<Struct::StaticModelPerObjectDrawRequestData>> m_forwardDrawRequestDataSmartPointerVectorList = {};

        FWK_DEFINE_TYPE_INFO(StaticModelPerObjectDrawRequestBase, DrawRequestPerObjectBase)
    };
}