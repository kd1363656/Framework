#pragma once

namespace FWK::Graphics
{
    // 大量のSkeletalAnimationModelを、GPUスキニング + GPUドリブンで描画するクラス
    //
    // 描画の流れ :
    // 1. 初めて使うモデルは、全モーションをベイク(事前計算)してスキニング行列をGPUへ置く
    // 2. 描画申請ごとに「何コマ目のアニメーションか」を決め、インスタンスの表を作る
    // 3. Passが描画用PipelineStateを設定する前に、DispatchGPUCullingでViewごとのカリングを行う
    // 4. Passが描画用PipelineStateと定数バッファを設定した後、SetupPerObjectConstantBufferでまとめて描画する
    //    (メッシュシェーダーが、見えている頂点だけをその場でスキニングする)
    class SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase : public DrawRequestPerObjectBase
    {
    public:

         SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase()          = default;
        ~SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase() override = default;

        void BeginFrame() override;

        // 現在のView(SetCurrentDrawViewIndexで指定)の描画コマンドを、ExecuteIndirectでまとめて実行する
        void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

        void AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelGPUSkinnedDrawRequestData>& a_drawRequestData);

        // 指定したViewについて、見えるMeshの選別とLOD選択をGPUで行い、描画コマンドを作成する
        // ※ ComputeShader用のPipelineStateを設定するため、描画用PipelineStateを設定する前に呼ぶこと
        bool DispatchGPUCulling(Renderer& a_renderer, const std::size_t a_viewIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView);

        void SetCurrentDrawViewIndex(const std::size_t a_set) { m_currentDrawViewIndex = a_set; }

        // GPUが数えた、描画した三角形の総数(LOD適用後、数フレーム前の結果、計測用)
        auto GetVALSubmittedTriangleCount() const { return m_gpuDrivenDrawer.GetVALSubmittedTriangleCount(); }

    private:

        // モデルのベイク済みアニメーションを取得する(まだ無ければベイクして作成する)
        std::shared_ptr<SkeletalAnimationBakedSkinning> FindOrCreateBakedSkinning(SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const;

        // インスタンスだけのモーフのウェイトを、モデルのチャンネル数に合わせ(足りない分は0)、エディターで設定した範囲へ収めてから表へ追加する
        // 戻り値は表の中での開始位置(入りきらない場合は無効値)
        std::uint32_t AddMorphWeightList(const SkeletalAnimationBakedSkinning& a_bakedSkinning, const std::vector<float>& a_morphWeightList);

        static constexpr std::size_t k_defaultDrawViewIndex = 0ULL;

        Utility::SmartPointerVectorList<std::weak_ptr<Struct::SkeletalAnimationModelGPUSkinnedDrawRequestData>> m_drawRequestDataSmartPointerVectorList = {};

        ModelGPUDrivenTableBuilder m_gpuDrivenTableBuilder = {};
        ModelGPUDrivenDrawer       m_gpuDrivenDrawer       = {};

        std::vector<float> m_morphWeightWorkList = {};

        std::size_t m_currentDrawViewIndex = k_defaultDrawViewIndex;

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase, DrawRequestPerObjectBase)
    };
}