#pragma once

namespace FWK::Graphics
{
    class Renderer final
    {
    private:

        using RootSignatureMap = std::unordered_map<Enum::RootSignatureType, std::shared_ptr<RootSignature>>;
        using PipelineStateMap = std::unordered_map<Enum::PipelineStateType, std::shared_ptr<PipelineStateBase>>;

    public:

         Renderer() = default;
        ~Renderer() = default;

        void INIT       ();
        void Deserialize(const nlohmann::json& a_rootJson);

        bool PostDeserialize(const Device&                   a_device,
                             const Window&                   a_window,
                             const Factory&                  a_factory,
                             const Struct::WindowClientSize& a_clientSize,
                                   ResourceContext&          a_resourceContext);

        void BeginFrame(const ResourceContext& a_resourceContext);
        void Execute   (const ResourceContext& a_resourceContext);
        void EndFrame  ();

        nlohmann::json Serialize() const;

        void Resize(const Device& a_device, const Struct::WindowClientSize& a_clientSize, ResourceContext& a_resourceContext);

        void RequestPreviewRenderView(const std::weak_ptr<Struct::CBCameraPass>& a_camera, const Struct::WindowClientSize& a_previewClientSize);

        void PreparePreviewRenderView(const Device& a_device, ResourceContext& a_resourceContext);

        void AddFrameResource(const std::shared_ptr<FrameResource>&     a_frameResource);
        void AddRootSignature(const std::shared_ptr<RootSignature>&     a_rootSignature, const Enum::RootSignatureType a_rootSignatureType);
        void AddPipelineState(const std::shared_ptr<PipelineStateBase>& a_pipelineState, const Enum::PipelineStateType a_pipelineStateType);

        std::weak_ptr<RootSignature> FindVALRootSignature(const Enum::RootSignatureType a_rootSignatureType) const;

        template <Concept::IsDerivedPipelineStateBaseConcept PipelineStateType>
        std::weak_ptr<PipelineStateType> FindVALPipelineState(const Enum::PipelineStateType a_pipelineStateType) const
        {
            const auto& l_pipelineStateITR = m_pipelineStateMap.find(a_pipelineStateType);

            if (l_pipelineStateITR == m_pipelineStateMap.end()) { return {}; }

            auto& l_pipelineState = l_pipelineStateITR->second;

            if (!l_pipelineState) { return {}; }


            const auto* l_currentINFO  = &l_pipelineState->GetREFRuntimeTypeINFO();
            const auto& l_baseTypeINFO = PipelineStateType::GetREFTypeINFO      ();
                  bool  l_isDeriveFrom = false;

            while (l_currentINFO)
            {
                if (l_currentINFO->k_staticTypeID == l_baseTypeINFO.k_staticTypeID)
                {
                    l_isDeriveFrom = true;
                }

                l_currentINFO = l_currentINFO->k_baseINFO;
            }

            // もし基底クラスの型情報と、派生クラスの型情報が一致したらキャスト
            // 一致しなければreturn
            if (!l_isDeriveFrom) { return {}; }

            return std::static_pointer_cast<PipelineStateType>(l_pipelineState);
        }

        const RenderView& FetchREFActiveRenderView() const;

        const auto& GetREFFrameResourceList() const { return m_frameResourceList; }
        const auto& GetREFRootSignatureMap () const { return m_rootSignatureMap; }
        const auto& GetREFPipelineStateMap () const { return m_pipelineStateMap; }

        const auto& GetREFCurrentFrameResource() const { return m_currentFrameResource; }

        const auto& GetREFSwapChain     () const { return m_swapChain; }
        const auto& GetREFShadowContext () const { return m_shadowContext; }
        const auto& GetREFMainRenderView() const { return m_mainRenderView; }

        const auto& GetREFRenderGraph() const { return m_renderGraph; }

        const auto& GetREFDirectCommandQueue() const { return m_directCommandQueue; }
        const auto& GetREFDirectCommandList () const { return m_directCommandList; }

        const auto& GetREFComputeCommandQueue() const { return m_computeCommandQueue; }
        const auto& GetREFComputeCommandList () const { return m_computeCommandList; }

        const auto& GetREFCurrentFrameResourceIndex() const { return m_currentFrameResourceIndex; }

        const auto& GetREFPreviewClientSize() const { return m_previewClientSize; }

        auto& GetMutableREFSwapChain  () { return m_swapChain; }
        auto& GetMutableREFRenderGraph() { return m_renderGraph; }

        auto& GetMutableREFShadowContext () { return m_shadowContext; }
        auto& GetMutableREFMainRenderView() { return m_mainRenderView; }

        auto& GetMutableREFDirectCommandList () { return m_directCommandList; }
        auto& GetMutableREFComputeCommandList() { return m_computeCommandList; }

        bool GetVALIsPreviewViewActive() const { return m_isPreviewViewActive; }

    private:

        bool SetupScreenRenderArea(const Struct::WindowClientSize& a_clientSize);

        void ResetCommandObjects(const FrameResource& a_frameResource);

        void DecideNextFrameUseFrameResource();

        bool PrepareForSwapChainResize();

        void SyncSpritePassDrawRequest();

        void ExecutePreviewView(const ResourceContext& a_resourceContext, FrameResource& a_frameResource);

        static constexpr std::size_t k_initialFrameResourceIndex   = 0ULL;
        static constexpr std::size_t k_frameResourceIndexIncrement = 1ULL;

        RootSignatureMap m_rootSignatureMap = {};
        PipelineStateMap m_pipelineStateMap = {};

        std::vector<std::shared_ptr<FrameResource>> m_frameResourceList = {};

        std::shared_ptr<Struct::CBSpritePass> m_cbSpritePass = nullptr;

        std::weak_ptr<FrameResource> m_currentFrameResource = {};

        SwapChain     m_swapChain         = {};
        ShadowContext m_shadowContext     = {};
        RenderView    m_mainRenderView    = {};
        RenderView    m_previewRenderView = {};

        TypeAlias::DirectCommandQueue  m_directCommandQueue  = {};
        TypeAlias::ComputeCommandQueue m_computeCommandQueue = {};
        DirectCommandList              m_directCommandList   = {};
        ComputeCommandList             m_computeCommandList  = {};

        RenderGraph m_renderGraph = {};

        Converter::RendererJsonConverter m_jsonConverter = {};

        Struct::WindowClientSize m_previewClientSize = {};

        std::size_t m_currentFrameResourceIndex = k_initialFrameResourceIndex;

        bool m_isPreviewRequested  = false;
        bool m_isPreviewViewActive = false;
    };
}