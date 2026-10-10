#include "ModelRenderTableUploadPass.h"

FWK::Graphics::ModelRenderTableUploadPass::ModelRenderTableUploadPass()
{
    // すべてのモデルのパス(スキニング・影・Lit / UnLit)より前に実行する
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Upload);
}
FWK::Graphics::ModelRenderTableUploadPass::~ModelRenderTableUploadPass() = default;

void FWK::Graphics::ModelRenderTableUploadPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph&)
{
    const auto& l_modelRenderSystem = a_renderer.GetREFModelRenderSystem        ();
    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList        ();
    const auto& l_frameIndex        = a_renderer.GetREFCurrentFrameResourceIndex();

    // 書き換えた要素だけを、このフレームのUPLOADバッファ経由でテーブル本体へコピーする命令を積む
    // コピーとモデルのパスは同じダイレクトコマンドリストに積むので、GPUは積んだ順(コピー → 描画)に実行する
    l_modelRenderSystem.RecordUpload(l_directCommandList, l_frameIndex);
}