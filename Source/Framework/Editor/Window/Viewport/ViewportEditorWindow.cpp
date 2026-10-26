#include "ViewportEditorWindow.h"

FWK::Editor::ViewportEditorWindow::ViewportEditorWindow() :
    m_imGuiSRVDescriptorIndexList(),

    m_toolbar       (),
    m_overlayToolbar(),
    m_debugCamera   (),
    m_cameraPreview (),
    m_gizmo         (),
    m_picker        (),

    m_jsonConverter(),

    m_gizmoOperation(Enum::ViewportGizmoOperation::Translate),

    m_viewportAspectRatio(Constant::k_viewportInvalidAspectRatio),

    m_isViewportHoveredPreviousFrame(false)
{}
FWK::Editor::ViewportEditorWindow::~ViewportEditorWindow()
{
    auto& l_editorManager = EditorManager::GetInstance();

    for (const auto& l_srvDescriptorIndex : m_imGuiSRVDescriptorIndexList)
    {
        l_editorManager.ReleaseImGuiSRVDescriptorIndex(l_srvDescriptorIndex);
    }

    m_imGuiSRVDescriptorIndexList.clear();
}

void FWK::Editor::ViewportEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Editor::ViewportEditorWindow::PostDeserialize()
{
    SetupViewportTextureDescriptors();
}

void FWK::Editor::ViewportEditorWindow::Draw(EditorManager& a_editorManager)
{
    // 前のフレームでマウスがビューポートの絵の上にあった場合は、ウィンドウを動かせないようにする
    // (絵の上のドラッグは、ギズモの操作やデバッグカメラの操作に使うため。ウィンドウはタイトルバーで動かせる)
    const ImGuiWindowFlags l_windowFlags = m_isViewportHoveredPreviousFrame ? ImGuiWindowFlags_NoMove : ImGuiWindowFlags_None;

    // Viewport用のImGuiウィンドウを開始する
    if (!ImGui::Begin(k_editorName.data(), nullptr, l_windowFlags))
    {
        m_isViewportHoveredPreviousFrame = false;

        ImGui::End();

        return;
    }

    ReportActiveWindowIfMouseClicked(a_editorManager);

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    // Viewport画像より先にツールバーを書く。
    // これにより画面へ重ならず、Viewport上部へ工程表示される
    m_toolbar.Draw  (a_editorManager);
    ImGui::Separator();

    // 現在のViewportウィンドウ内で、実際に画像を表示できる領域サイズを取得する
    // Dockingでウィンドウサイズが変わると、この値も変わる
    const auto& l_viewportSize = ImGui::GetContentRegionAvail();

    if (l_viewportSize.x <= k_minViewportSize ||
        l_viewportSize.y <= k_minViewportSize)
    {
        // 無効サイズの場合は描画をしない
        ImGui::End();

        return;
    }

    // RenderTargetTextureの解像度はウィンドウサイズのまま、カメラの投影だけをViewportのAspectRatioに合わせる
    // (Textureへは横方向に縮んだ状態で描かれ、表示時にViewportの形へ引き伸ばされることで元の縦横比に戻る)
    m_viewportAspectRatio = l_viewportSize.x / l_viewportSize.y;

    // RenderGraphで作成された最終カラーTextureを、ImGuiで表示できるTextureIDとして取得する
    // 今回はRenderTargetTextureの解像度変更は行わず、既存の描画結果をViewport内に拡縮表示する
    const auto l_viewportTextureID = FetchVALViewportTextureID();

    // 無効なテクスチャなのでreturn
    if (l_viewportTextureID == k_invalidViewportTextureID)
    {
        ImGui::End();

        return;
    }

    // 絵を描く前のカーソル位置が、絵の左上の角になる
    const Struct::ViewportScreenRect l_viewportRect = { ImGui::GetCursorScreenPos(), l_viewportSize };

    // Viewportの表示領域全体に、取得したTextureを描画する
    DrawViewportTexture(l_viewportTextureID, l_viewportSize);

    // 絵の上に重ねる物を描く
    DrawViewportOverlay(a_editorManager, l_viewportRect);

    ImGui::End();
}

nlohmann::json FWK::Editor::ViewportEditorWindow::Serialize()
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::ViewportEditorWindow::OnDrawSkipped()
{
    auto& l_graphicsManager       = Graphics::GraphicsManager::GetInstance                                 ();
    auto& l_renderer              = l_graphicsManager.GetMutableREFRenderer                                ();
    auto& l_renderGraph           = l_renderer.GetMutableREFRenderGraph                                    ();
    auto  l_cameraPassDrawRequest = l_renderGraph.FindVALDrawRequestPass<Graphics::CameraPassDrawRequest>().lock();

    if (!l_cameraPassDrawRequest) { return; }

    // エディターを隠している間はゲームの画面として表示されるため、デバッグカメラでの描画をやめてメインカメラへ戻す
    m_debugCamera.ReleaseRenderView(*l_cameraPassDrawRequest);

    m_isViewportHoveredPreviousFrame = false;
}

void FWK::Editor::ViewportEditorWindow::SetupViewportTextureDescriptors()
{
    m_imGuiSRVDescriptorIndexList.clear();

    // GraphicsManagerからRendererへアクセスし、現在フレームのRendererGraphリソースを取得する
    const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
    const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();

    for (const auto& l_frameResource : l_renderer.GetREFFrameResourceList())
    {
        FWK_ASSERT_RETURN_IF(!l_frameResource, "フレームリソースの取得に失敗しており、PostDeserialize処理に失敗しました。");

        // フレームリソースで管理されている、レンダーグラフフレームリソースを取得
        const auto& l_renderGraphFrameResource = l_frameResource->GetREFRenderGraphFrameResource();

        // ファイナルカラーパステクスチャを取得
        // (実際にバックバッファに描画する際に使用するガンマ補正などを適用したレンダーターゲットテクスチャ)
        const auto& l_renderTargetPassTexture = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(Enum::RenderGraphRenderTargetType::FinalColor).lock();

        FWK_ASSERT_RETURN_IF(!l_renderTargetPassTexture, "レンダーターゲットパステクスチャの取得に失敗しており、PostDeserialize処理に失敗しました。");

        // RenderTargetTexture本体を取得する
        const auto& l_renderTargetTexture = l_renderTargetPassTexture->GetREFRenderTargetTexture();

        if (l_renderTargetTexture.GetVALSRVDescriptorIndex() == Graphics::DescriptorHeap::k_invalidDescriptorIndex) { return; }

        // ImGuiでTextureを表示するにはTextureをShaderから読めるSRVが必要になる
        // ここではRenderTargetTextureに割り当てられているSRVのDescriptorIndexを取得する
        const auto l_srvDescriptorIndex = l_renderTargetTexture.GetVALSRVDescriptorIndex();

        FWK_ASSERT_RETURN_IF(l_srvDescriptorIndex == Graphics::DescriptorHeap::k_invalidDescriptorIndex, "SRVDescriptorIndexが無効値になっており、PostDeserialize処理に失敗しました。");

        auto& l_editorManager = EditorManager::GetInstance();

        // SRVDescriptorIndexをアロケート
        m_imGuiSRVDescriptorIndexList.emplace_back(l_editorManager.AllocateImGuiSRVDescriptorIndex());

        const auto& l_resourceContext = l_graphicsManager.GetREFResourceContext();

        // メイン描画用SRVDescriptorを、ImGui用SRVDescriptorへコピーする
        const auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetREFCBVSRVUAVDescriptorPool();

        FWK_ASSERT_RETURN_IF(!l_editorManager.CopyGraphicsSRVDescriptor(l_cbvSRVUAVDescriptorPool, l_srvDescriptorIndex, m_imGuiSRVDescriptorIndexList.back()), "SRVDescriptorのコピー処理に失敗しました。");
    }
}

ImTextureID FWK::Editor::ViewportEditorWindow::FetchVALViewportTextureID() const
{
    const auto& l_editorManager = EditorManager::GetInstance();

    const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
    const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();

    const auto& l_currentFrameResourceIndex = l_renderer.GetREFCurrentFrameResourceIndex();

    // レンダーラーから現在のFrameResourceのインデックスを取得
    FWK_ASSERT_RETURN_VALUE_IF(m_imGuiSRVDescriptorIndexList.size() <= l_currentFrameResourceIndex, "フレームリソースの数がimGuiSRVDescriptorIndexListのサイズを超えています", k_invalidViewportTextureID);

    // もし無効なDescriptorIndexならreturn
    if (m_imGuiSRVDescriptorIndexList[l_currentFrameResourceIndex] == Graphics::DescriptorHeap::k_invalidDescriptorIndex) { return k_invalidViewportTextureID; }

    return l_editorManager.FetchVALImGuiTextureID(m_imGuiSRVDescriptorIndexList[l_currentFrameResourceIndex]);
}

void FWK::Editor::ViewportEditorWindow::DrawViewportTexture(const ImTextureID& a_textureID, const ImVec2& a_viewportSize) const
{
    // Texture全体を表示するため、UV範囲は左上から右下までを指定する
    const auto l_uvMIN = ImVec2(k_viewportUVMINX, k_viewportUVMINY);
    const auto l_uvMAX = ImVec2(k_viewportUVMAXX, k_viewportUVMAXY);

    // 取得したTextureをViewportの表示領域いっぱいに描画する
    // a_viewportSizeにGetContentRegionAvailの値を渡しているため、Dockingの拡縮に追従する
    // カメラはViewportのAspectRatioで投影しているため、ここで引き伸ばしても縦横比は崩れない
    ImGui::Image(a_textureID,
                 a_viewportSize,
                 l_uvMIN,
                 l_uvMAX);
}

void FWK::Editor::ViewportEditorWindow::DrawViewportOverlay(EditorManager& a_editorManager, const Struct::ViewportScreenRect& a_viewportRect)
{
    auto& l_graphicsManager       = Graphics::GraphicsManager::GetInstance                                 ();
    auto& l_renderer              = l_graphicsManager.GetMutableREFRenderer                                ();
    auto& l_renderGraph           = l_renderer.GetMutableREFRenderGraph                                    ();
    auto  l_cameraPassDrawRequest = l_renderGraph.FindVALDrawRequestPass<Graphics::CameraPassDrawRequest>().lock();

    if (!l_cameraPassDrawRequest) { return; }

    const bool l_isViewportHovered = FetchVALIsViewportHovered(a_viewportRect);

    m_isViewportHoveredPreviousFrame = l_isViewportHovered;

    // デバッグカメラを動かし、次のフレームの描画カメラ、カリングカメラを決める
    // (Rendererはこのウィンドウより先に描き終えているため、ここで決めた内容は次のフレームの描画に使われる)
    m_debugCamera.Update         (m_viewportAspectRatio, l_isViewportHovered);
    m_debugCamera.ApplyRenderView(*l_cameraPassDrawRequest);

    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();

    // 削除されたGameObjectを選択から外しておく(ギズモが消えた物を掴まないようにするため)
    l_gameObjectSelectionState.SweepUnavailableGameObjects();

    const auto& l_sceneManager             = SceneManager::GetInstance                          ();
    const auto& l_scene                    = l_sceneManager.GetVALScene                         ().lock();
    const auto& l_drawCameraConstantBuffer = l_cameraPassDrawRequest->FetchVALDrawConstantBuffer();
    const auto& l_mainCameraConstantBuffer = l_cameraPassDrawRequest->GetREFSourceConstantBuffer().lock();

    // 目印、視錐台、ギズモは、今ビューポートを描いているカメラから見た位置に描く
    if (l_scene &&
        l_drawCameraConstantBuffer)
    {
        auto* const l_drawList = ImGui::GetWindowDrawList();

        // ビューポートの外へはみ出して描かないようにする
        const ImVec2 l_viewportMax = { a_viewportRect.m_min.x + a_viewportRect.m_size.x,
                                       a_viewportRect.m_min.y + a_viewportRect.m_size.y };

        // PushClipRect(左上、右下、今の範囲と重なる部分だけにするか)
        l_drawList->PushClipRect(a_viewportRect.m_min, l_viewportMax, k_intersectWithCurrentClipRect);

        m_picker.Draw(*l_scene,
                      l_gameObjectSelectionState,
                      *l_drawCameraConstantBuffer,
                      a_viewportRect,
                      l_isViewportHovered);

        if (l_mainCameraConstantBuffer &&
            l_mainCameraConstantBuffer != l_drawCameraConstantBuffer)
        {
            m_debugCamera.DrawMainCameraFrustum(*l_drawList,
                                                a_viewportRect,
                                                *l_drawCameraConstantBuffer,
                                                *l_mainCameraConstantBuffer);
        }

        l_drawList->PopClipRect();

        m_gizmo.Draw(l_gameObjectSelectionState,
                     *l_drawCameraConstantBuffer,
                     a_viewportRect,
                     m_gizmoOperation);
    }

    // 選択中のGameObjectがカメラを持っていれば、そのカメラから見た絵を右下に表示する
    m_cameraPreview.Draw(FetchVALSelectedCameraComponent(l_gameObjectSelectionState), a_viewportRect);

    // ボタンは一番上に重ねるため最後に描く
    m_overlayToolbar.Draw(a_viewportRect, m_gizmoOperation, m_debugCamera);

    // 左クリック : 目印をクリックしたら選択、何も無い所なら選択の解除(Ctrl / Shiftで複数選択)
    // ボタン、ギズモ、プレビューの上、デバッグカメラの操作中のクリックは選択に使わない
    if (!l_isViewportHovered                          ||
        !ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
        ImGui::IsAnyItemHovered()                     ||
        m_gizmo.FetchVALIsOverOrUsing()               ||
        m_cameraPreview.FetchVALIsMouseOverPanel()    ||
        m_debugCamera.GetVALIsControlling())
    {
        return;
    }

    const auto& l_io = ImGui::GetIO();

    // Ctrl又はShiftを押しながらのクリックは、選択への追加 / 解除(複数選択)
    const bool l_isToggleSelection = l_io.KeyCtrl ||
                                     l_io.KeyShift;

    m_picker.SelectHoveredGameObject(a_editorManager, l_isToggleSelection);
}

bool FWK::Editor::ViewportEditorWindow::FetchVALIsViewportHovered(const Struct::ViewportScreenRect& a_viewportRect) const
{
    // このウィンドウがマウスの下にあり(他のウィンドウやポップアップが重なっていない)、かつ絵の範囲の中にマウスがあるか
    const ImVec2 l_viewportMax = { a_viewportRect.m_min.x + a_viewportRect.m_size.x,
                                   a_viewportRect.m_min.y + a_viewportRect.m_size.y };

    return ImGui::IsWindowHovered()                                     &&
           ImGui::IsMouseHoveringRect(a_viewportRect.m_min, l_viewportMax);
}

std::weak_ptr<FWK::GameObjectCameraComponent> FWK::Editor::ViewportEditorWindow::FetchVALSelectedCameraComponent(const EditorGameObjectSelectionState& a_gameObjectSelectionState) const
{
    // 最後に選んだGameObjectがCameraComponentを持っていれば、それを返す
    const auto& l_gameObject = a_gameObjectSelectionState.FindVALLastSelectedGameObject().lock();

    if (!l_gameObject) { return {}; }

    const auto& l_componentContainer = l_gameObject->GetREFComponentContainer();

    return l_componentContainer.FindUniqueComponent<GameObjectCameraComponent>();
}
