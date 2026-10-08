#include "RenderGraph.h"

void FWK::Graphics::RenderGraph::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Graphics::RenderGraph::Compile()
{
    // Passの依存関係を解決し、実行順に並び替える
    m_passSorter.SortPassList(m_passList);
}

void FWK::Graphics::RenderGraph::BeginFrame(const ResourceContext& a_resourceContext, Renderer& a_renderer)
{
    // 使用、ポインタがnullのパスの削除
    RemoveExpiredPassList();

    for (const auto& l_computeRequestPerObject : m_computeRequestPerObjectList)
    {
        FWK_ASSERT_RETURN_IF(!l_computeRequestPerObject, "ComputeRequestPerObjectが無効のため、BeginFrame処理に失敗しました。");

        l_computeRequestPerObject->BeginFrame();
    }

    for (const auto& l_drawRequestPerObject : m_drawRequestPerObjectList)
    {
        FWK_ASSERT_RETURN_IF(!l_drawRequestPerObject, "DrawRequestPerObjectが無効のため、BeginFrame処理に失敗しました。");

        l_drawRequestPerObject->BeginFrame();
    }

    // バックバッファのリソース状態をRENDER_TARGETに遷移、バックバッファのクリア、RTVのセットを行う
    BeginBackBuffer(a_resourceContext, a_renderer);

    // 現在のフレームリソースが持つPassTextureをClearする
    m_resourceClearer.ClearCurrentFramePassTextureList(a_resourceContext, a_renderer);
}
void FWK::Graphics::RenderGraph::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer)
{
    SetupDescriptorHeap(a_resourceContext, a_renderer);

    // メインビューのテクスチャは、BeginFrameでクリア済み
    // パスを実行順(m_passListはCompileで実行順に並べ替え済み)に1つずつ実行する
    // メインビューでは、スキニングのコンピュートや画面に出す最後のパスのように、
    // メインビューだけで実行するパスも含めて、すべてのパスを実行する
    for (const auto& l_pass : m_passList)
    {
        if (!l_pass) { continue; }

        ExecutePass(a_resourceContext, *l_pass, a_renderer);
    }
}
void FWK::Graphics::RenderGraph::ExecutePreviewView(const ResourceContext& a_resourceContext, Renderer& a_renderer)
{
    const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

    FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のFrameResourceが無効のため、プレビューの描画に失敗しました。");

    // 描画の対象を、プレビュー用のテクスチャ一式へ切り替える
    // パスの遷移・クリア・描画先の設定は、この切り替えの後、プレビュー用のFinalColorなどに対して行われる
    l_currentFrameResource->SetActiveViewType(Enum::RenderViewType::Preview);

    SetupDescriptorHeap(a_resourceContext, a_renderer);

    // 前のフレームで、ImGuiが読めるようPIXEL_SHADER_RESOURCEにしたFinalColorを、
    // クリアと書き込みができるRENDER_TARGETへ戻す
    // (すでにRENDER_TARGETなら、何もしない)
    m_resourceTransitioner.TransitionRenderTargetPassTexture(a_renderer, Enum::RenderGraphRenderTargetType::FinalColor, Enum::RenderGraphResourceUsage::RenderTarget);

    // メインビューのテクスチャはBeginFrameでクリア済みだが、プレビューのテクスチャはここでクリアする
    m_resourceClearer.ClearCurrentFramePassTextureList(a_resourceContext, a_renderer);

    for (const auto& l_pass : m_passList)
    {
        if (!l_pass) { continue; }

        // 「すべてのビューで実行する(AllViews)」と宣言されたパスだけを実行する
        // 宣言されていないパスは、メインビューだけで実行するパスとして扱い、ここでは飛ばす
        // 例 : エディター専用の線の描画は、プレビューには描かない
        if (l_pass->GetVALViewScope() != Enum::RenderGraphPassViewScope::AllViews) { continue; }

        ExecutePass(a_resourceContext, *l_pass, a_renderer);
    }

    // 描き終えたFinalColorを、ImGuiが画面に表示できる状態にして終える
    // ※注意 : 遷移の対象は「今のビューのFinalColor」なので、メインビューへ戻す前に行う
    m_resourceTransitioner.TransitionRenderTargetPassTexture(a_renderer, Enum::RenderGraphRenderTargetType::FinalColor, Enum::RenderGraphResourceUsage::PixelShaderResource);

    // 描画が終わったので、メインビューへ戻す
    // (ビューポートの表示などが、メインビューのテクスチャを参照するため)
    l_currentFrameResource->SetActiveViewType(Enum::RenderViewType::Main);

    // 描画先もバックバッファへ戻す
    // ※注意 : ビューポートとシザー矩形は「今のビュー」の描画領域から設定されるため、
    //          必ずメインビューへ戻した後に呼ぶ
    RestoreBackBufferRenderTarget(a_resourceContext, a_renderer);
}
void FWK::Graphics::RenderGraph::EndFrame(Renderer& a_renderer) const
{
          auto& l_swapChain         = a_renderer.GetMutableREFSwapChain ();
    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    const auto& l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
          auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList   ();

    FWK_ASSERT_RETURN_IF(l_backBufferList.empty(),                                        "BackBufferListが空のため、BackBufferのPresent遷移に失敗しました。");
    FWK_ASSERT_RETURN_IF(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのPresent遷移に失敗しました。");

    auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

    FWK_ASSERT_RETURN_IF(l_backBuffer.m_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのPresent遷移に失敗しました。");

    // BackBufferをRENDERTARGET -> PRESENTへ遷移
    // ImGuiとの連携の関係上明示的にリソース遷移を行う
    m_resourceTransitioner.TransitionBackBufferResource(l_directCommandList, D3D12_RESOURCE_STATE_PRESENT, l_backBuffer);
}

nlohmann::json FWK::Graphics::RenderGraph::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::RenderGraph::AddPass(std::unique_ptr<RenderGraphPassBase>&& a_pass)
{
    FWK_ASSERT_RETURN_IF(!a_pass, "RenderGraphPassが無効のため、PassListへの登録処理に失敗しました。");

    m_passList.emplace_back(std::move(a_pass));
}

void FWK::Graphics::RenderGraph::AddDrawRequestPass(const std::shared_ptr<DrawRequestPassBase>& a_drawRequestPass)
{
    FWK_ASSERT_RETURN_IF(!a_drawRequestPass, "DrawRequestPassが無効のため、DrawRequestPassListへの登録に失敗しました。");

    const auto l_staticTypeID = a_drawRequestPass->GetREFRuntimeTypeINFO().k_staticTypeID;

    FWK_ASSERT_RETURN_IF(m_drawRequestPassMap.contains(l_staticTypeID), "同じ型のDrawRequestPassを二重登録しようとしており、DrawRequestPassMapへの登録に失敗しました。");

    m_drawRequestPassList.emplace_back(a_drawRequestPass);
    m_drawRequestPassMap.try_emplace  (l_staticTypeID, a_drawRequestPass);
}
void FWK::Graphics::RenderGraph::AddComputeRequestPerObject(const std::shared_ptr<ComputeRequestPerObjectBase>& a_computeRequestPerObject)
{
    FWK_ASSERT_RETURN_IF(!a_computeRequestPerObject, "ComputeRequestPerObjectが無効のため、ComputeRequestPerObjectListへの登録に失敗しました。");

    const auto l_staticTypeID = a_computeRequestPerObject->GetREFRuntimeTypeINFO().k_staticTypeID;

    FWK_ASSERT_RETURN_IF(m_computeRequestPerObjectMap.contains(l_staticTypeID), "同じ型のComputeRequestPerObjectを二重登録しようとしており、ComputeRequestPerObjectMapへの登録に失敗しました。");

    m_computeRequestPerObjectList.emplace_back(a_computeRequestPerObject);
    m_computeRequestPerObjectMap.try_emplace  (l_staticTypeID, a_computeRequestPerObject);
}
void FWK::Graphics::RenderGraph::AddDrawRequestPerObject(const std::shared_ptr<DrawRequestPerObjectBase>& a_drawRequestPerObject)
{
    FWK_ASSERT_RETURN_IF(!a_drawRequestPerObject, "DrawRequestPerObjectsが無効のため、DrawRequestPerObjectListへの登録に失敗しました。");

    const auto l_staticTypeID = a_drawRequestPerObject->GetREFRuntimeTypeINFO().k_staticTypeID;

    FWK_ASSERT_RETURN_IF(m_drawRequestPerObjectMap.contains(l_staticTypeID), "同じ型のDrawRequestPerObjectを二重登録しようとしており、DrawRequestPerObjectMapへの登録に失敗しました。");

    m_drawRequestPerObjectList.emplace_back(a_drawRequestPerObject);
    m_drawRequestPerObjectMap.try_emplace  (l_staticTypeID, a_drawRequestPerObject);
}

void FWK::Graphics::RenderGraph::BeginBackBuffer(const ResourceContext& a_resourceContext, Renderer& a_renderer) const
{
          auto& l_swapChain         = a_renderer.GetMutableREFSwapChain        ();
    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList       ();
    const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();

    const auto& l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
          auto& l_backBufferList  = l_swapChain.GetMutableREFBackBufferList   ();

    FWK_ASSERT_RETURN_IF(l_backBufferList.empty(), "BackBufferListが空のため、BackBufferのClearに失敗しました。");
    FWK_ASSERT_RETURN_IF(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "BackBufferIndexが範囲外のため、BackBufferのClearに失敗しました。");

    auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

    FWK_ASSERT_RETURN_IF(l_backBuffer.m_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "BackBufferのRTVDescriptorIndexが無効のため、BackBufferのClearに失敗しました。");

    // BackBufferをPRESENT -> RENDERTARGETに明示的に遷移
    m_resourceTransitioner.TransitionBackBufferResource(l_directCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET, l_backBuffer);

    // このバックバッファを描画先として設定する
    l_directCommandList.SetupRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);

    // 描画先に設定したBackBufferを指定色でClearする
    l_directCommandList.ClearRenderTarget(l_rtvDescriptorPool, l_backBuffer.m_rtvDescriptorIndex);
}
void FWK::Graphics::RenderGraph::RestoreBackBufferRenderTarget(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const
{
    // メイン以外のビュー(プレビュー)を描いた後は、最後のパスが設定した
    // そのビュー用のテクスチャが、描画先(OMのレンダーターゲット)に残ったままになっている
    // この後に描くImGuiは描画先を自分で設定しないため、メインビューの最後と同じバックバッファへ戻す
    m_resourceBinder.SetupBackBufferRenderTarget(a_resourceContext, a_renderer);
}

void FWK::Graphics::RenderGraph::SetupDescriptorHeap(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const
{
    const auto& l_cbvSRVUAVDescriptorPool = a_resourceContext.GetREFCBVSRVUAVDescriptorPool();
    const auto& l_directCommandList       = a_renderer.GetREFDirectCommandList             ();
    const auto& l_computeCommandList      = a_renderer.GetREFComputeCommandList            ();

    // GraphicsShaderからBindlessResourceを参照するため、
    // Direct Command ListへShaderVisibleDescriptorHeapを設定する
    l_directCommandList.SetupDescriptorHeap(l_cbvSRVUAVDescriptorPool);

    // ComputeShaderも同じDescriptor Heap内の
    // SRV/UAVDescriptorIndexを参照するため、
    // ComputeCommandListへも設定する
    l_computeCommandList.SetupDescriptorHeap(l_cbvSRVUAVDescriptorPool);
}

void FWK::Graphics::RenderGraph::ExecutePass(const ResourceContext& a_resourceContext, RenderGraphPassBase& a_pass, Renderer& a_renderer)
{
    // Pass実行前に、ResourceAccessのbeforeUsageへ遷移する
    m_resourceTransitioner.TransitionPassResourceBefore(a_pass, a_renderer);

    // PassのWriteResourceを見て、RenderGraph側でRTVを自動セットする
    m_resourceBinder.SetupPassRenderTarget(a_resourceContext, a_pass, a_renderer);

    a_pass.Execute(a_resourceContext, a_renderer, *this);

    // Pass実行後に、ResourceAccessのafterUsageへ遷移する
    m_resourceTransitioner.TransitionPassResourceAfter(a_pass, a_renderer);
}

void FWK::Graphics::RenderGraph::RemoveExpiredPassList()
{
    // nullptrのパスを取り除く
    // 末尾の要素と入れ替えて消すと、Compileで決めた実行順が崩れてしまうため、
    // 残る要素の並びを保ったまま消すstd::erase_ifを使う
    std::erase_if(m_passList,
                  [](const auto& a_pass)
                  {
                      return !a_pass;
                  });
}