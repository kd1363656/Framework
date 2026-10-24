#pragma once

namespace FWK::Graphics
{
    // StaticModelの描画申請をまとめ、GPUドリブン(GPU主導)で描画するクラス
    //
    // 描画の流れ :
    // 1. Passが描画用PipelineStateを設定する前に、DispatchGPUCullingでViewごとのカリングを行う
    // 2. Passが描画用PipelineStateと定数バッファを設定した後、SetupPerObjectConstantBufferでまとめて描画する
    class StaticModelPerObjectDrawRequestBase : public DrawRequestPerObjectBase
    {
    public:

         StaticModelPerObjectDrawRequestBase()          = default;
        ~StaticModelPerObjectDrawRequestBase() override = default;

        void BeginFrame() override;

        // 現在のView(SetCurrentDrawViewIndexで指定)の描画コマンドを、ExecuteIndirectでまとめて実行する
        void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

        void AddDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData);

        // 指定したViewについて、見えるMeshの選別とLOD選択をGPUで行い、描画コマンドを作成する
        // ※ ComputeShader用のPipelineStateを設定するため、描画用PipelineStateを設定する前に呼ぶこと
        bool DispatchGPUCulling(Renderer& a_renderer, const std::size_t a_viewIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView);

        void SetCurrentDrawViewIndex(const std::size_t a_set) { m_currentDrawViewIndex = a_set; }

        // GPUが数えた、描画した三角形の総数(LOD適用後、数フレーム前の結果、計測用)
        auto GetVALSubmittedTriangleCount() const { return m_gpuDrivenDrawer.GetVALSubmittedTriangleCount(); }

    private:

        static constexpr std::size_t k_defaultDrawViewIndex = 0ULL;

        Utility::SmartPointerVectorList<std::weak_ptr<Struct::StaticModelPerObjectDrawRequestData>> m_forwardDrawRequestDataSmartPointerVectorList = {};

        // 描画申請から1フレーム分の表を作るクラスと、それを使ってGPUドリブンで描画するクラス
        ModelGPUDrivenTableBuilder m_gpuDrivenTableBuilder = {};
        ModelGPUDrivenDrawer       m_gpuDrivenDrawer       = {};

        std::size_t m_currentDrawViewIndex = k_defaultDrawViewIndex;

        FWK_DEFINE_TYPE_INFO(StaticModelPerObjectDrawRequestBase, DrawRequestPerObjectBase)
    };
}