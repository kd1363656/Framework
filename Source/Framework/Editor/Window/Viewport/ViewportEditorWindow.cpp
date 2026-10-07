#include "ViewportEditorWindow.h"
#include "../../../../Application/Application.h"

FWK::Editor::ViewportEditorWindow::ViewportEditorWindow() :
    m_imGuiSRVDescriptorIndexList(),
    m_previewImGuiSRVDescriptorIndexList(),

    m_editorCamera(std::make_unique<EditorCamera>()),

    m_toolbar(),

    m_jsonConverter(),

    m_isDrawFrustum     (false),
    m_isDrawCulledResult(false)
{}
FWK::Editor::ViewportEditorWindow::~ViewportEditorWindow()
{
    // 確保したImGui用SRVDescriptorIndexをすべて返却する
    // (SetupViewportTextureDescriptorsでも同じ返却処理を使うため関数にまとめている)
    ReleaseViewportTextureDescriptors();
}

void FWK::Editor::ViewportEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::ViewportEditorWindow::PostDeserialize()
{
    FWK_ASSERT_RETURN_IF(!m_editorCamera, "エディターカメラが無効です。");

    const auto& l_application = Application::GetInstance  ();
    const auto& l_window      = l_application.GetREFWindow();

    m_editorCamera->Setup(l_window.GetVALAspectRatio());

    // エディターカメラをデバッグカメラとして登録する
    // エディターカメラの定数バッファはウィンドウが生きている間は変わらないため、ここで1回だけ登録する
    RegisterDebugCamera();

    SetupViewportTextureDescriptors();
}

void FWK::Editor::ViewportEditorWindow::Draw(EditorManager& a_editorManager)
{
    // Viewport用のImGuiウィンドウを開始する
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

    ReportActiveWindowIfMouseClicked(a_editorManager);

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    // Viewport画像より先にツールバーを書く。
    // これにより画面へ重ならず、Viewport上部へ工程表示される
    m_toolbar.Draw  (*this);
    ImGui::Separator();

    // 現在のViewportウィンドウ内で、実際に画像を表示できる領域サイズを取得する
    // Dockingでウィンドウサイズが変わると、この値も変わる
    const ImVec2& l_viewportSize = ImGui::GetContentRegionAvail();

    if (l_viewportSize.x <= k_minViewportSize ||
        l_viewportSize.y <= k_minViewportSize)
    {
        // 無効サイズの場合は描画をしない
        ImGui::End();

        return;
    }

    // Viewport表示領域のアスペクト比にエディターカメラを追従させる
    if (m_editorCamera)
    {
        m_editorCamera->ApplyAspectRatio(l_viewportSize.x / l_viewportSize.y);
    }

    // RenderGraphで作成された最終カラーTextureを、ImGuiで表示できるTextureIDとして取得する
    // 今回はRenderTargetTextureの解像度変更は行わなず、既存の描画結果をViewport内に拡縮表示する
    const auto& l_viewportTextureID = FetchVALViewportTextureID();

    // 無効なテクスチャなのでreturn
    if (l_viewportTextureID == k_invalidViewportTextureID)
    {
        ImGui::End();

        return;
    }

    // Viewportの表示領域全体に、取得したTextureを描画する
    DrawViewportTexture(l_viewportSize, l_viewportTextureID);

    // このフレームにプレビューを描いていれば、ビューポート画像の右下に重ねて表示する
    // (直前のItemがビューポート画像のため、その範囲を基準に位置を決める)
    DrawCameraPreview();

    // カメラを持つGameObjectを選択している間は、そのカメラの映像をプレビューとして描くようRendererへ依頼する
    RequestCameraPreview(a_editorManager, l_viewportSize);

    // Viewport画像上での入力(右クリック中の移動・回転)は、エディターカメラ自身が処理する
    // ビューポート画像の上にマウスがあるか(直前のItemがビューポート画像)は、ウィンドウ側が判断して渡す
    if (m_editorCamera)
    {
        m_editorCamera->HandleInput(ImGui::IsItemHovered());
    }

    ImGui::End();
}

nlohmann::json FWK::Editor::ViewportEditorWindow::Serialize()
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::ViewportEditorWindow::SetupViewportTextureDescriptors()
{
    // 以前確保したImGui用SRVDescriptorIndexを返却してから確保し直す
    // この関数はウィンドウのリサイズごとにEditorManager::ProcessWindowResizeRequestから呼ばれるため
    // clear()だけだと確保済みIndexがPoolへ返却されないまま新しいIndexを確保し続け、
    // リサイズを繰り返すとImGui用SRVDescriptorPoolを使い切ってしまう
    // (FrameResource3個分 × 約850回のリサイズで2560個を使い切る)
    // リサイズ時はRenderer::Resize内でGPUの完了を待っているため、
    // 返却したIndexがすぐ再利用されても描画中のDescriptorを上書きすることはない
    ReleaseViewportTextureDescriptors();

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

        // プレビュー用のImGui用SRVDescriptorIndexも、フレームリソースごとにアロケートしておく
        // プレビューのテクスチャは作成やサイズ変更のたびにSRVが変わるため、
        // コピーは作成時ではなく、表示する毎フレームに行う(DrawCameraPreview)
        m_previewImGuiSRVDescriptorIndexList.emplace_back(l_editorManager.AllocateImGuiSRVDescriptorIndex());

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

void FWK::Editor::ViewportEditorWindow::DrawViewportTexture(const ImVec2& a_viewportSize, const ImTextureID& a_textureID) const
{
    // Texture全体を表示するため、UV範囲は左上から右下までを指定する
    const auto& l_uvMIN = ImVec2(k_viewportUVMINX, k_viewportUVMINY);
    const auto& l_uvMAX = ImVec2(k_viewportUVMAXX, k_viewportUVMAXY);

    // 取得したTextureをViewportの表示領域いっぱいに描画する
    // a_viewportSizeにGetContentRegionAvailの値を渡しているため、Dockingの拡縮に追従する
    ImGui::Image(a_textureID,
                 a_viewportSize,
                 l_uvMIN,
                 l_uvMAX);
}

void FWK::Editor::ViewportEditorWindow::DrawCameraPreview() const
{
    const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
    const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();

    // このフレームにプレビューを描いていなければ、何も表示しない
    // (カメラを選択していない、選択を外した直後などは、右下に何も描かない)
    if (!l_renderer.GetVALIsPreviewViewActive()) { return; }

    // 現在のフレームリソースが、このフレームのプレビューを描いている
    const auto& l_currentFrameResourceIndex = l_renderer.GetREFCurrentFrameResourceIndex();
    const auto& l_frameResourceList         = l_renderer.GetREFFrameResourceList        ();

    FWK_ASSERT_RETURN_IF(l_currentFrameResourceIndex >= m_previewImGuiSRVDescriptorIndexList.size(), "フレームリソースの数がプレビュー用ImGuiSRVDescriptorIndexListのサイズを超えているため、プレビューの表示に失敗しました。");
    FWK_ASSERT_RETURN_IF(l_currentFrameResourceIndex >= l_frameResourceList.size(),                  "現在のフレームリソースのインデックスが範囲外のため、プレビューの表示に失敗しました。");

    const auto& l_frameResource = l_frameResourceList[l_currentFrameResourceIndex];

    FWK_ASSERT_RETURN_IF(!l_frameResource, "現在のフレームリソースが無効のため、プレビューの表示に失敗しました。");

    // プレビューのFinalColor(ガンマ補正済みの最終的な色)のテクスチャを取得する
    const auto& l_previewRenderGraphFrameResource = l_frameResource->GetREFPreviewRenderGraphFrameResource          ();
    const auto& l_finalColorPassTexture           = l_previewRenderGraphFrameResource.FindVALRenderTargetPassTexture(Enum::RenderGraphRenderTargetType::FinalColor).lock();

    if (!l_finalColorPassTexture) { return; }

    const auto& l_renderTargetTexture = l_finalColorPassTexture->GetREFRenderTargetTexture();
    const auto  l_srvDescriptorIndex  = l_renderTargetTexture.GetVALSRVDescriptorIndex    ();

    if (l_srvDescriptorIndex == Graphics::DescriptorHeap::k_invalidDescriptorIndex) { return; }

    // プレビューのSRVを、ImGui用のSRVへコピーする
    // プレビューのテクスチャは作成・サイズ変更でSRVが変わり得るため、表示する毎フレームにコピーする
    // このフレームリソース用のImGui用SRVは、GPUが前回の使用を終えているため、書き換えても安全
    const auto& l_editorManager           = EditorManager::GetInstance                     ();
    const auto& l_resourceContext         = l_graphicsManager.GetREFResourceContext        ();
    const auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetREFCBVSRVUAVDescriptorPool();
    const auto  l_imGuiSRVDescriptorIndex = m_previewImGuiSRVDescriptorIndexList[l_currentFrameResourceIndex];

    FWK_ASSERT_RETURN_IF(!l_editorManager.CopyGraphicsSRVDescriptor(l_cbvSRVUAVDescriptorPool, l_srvDescriptorIndex, l_imGuiSRVDescriptorIndex), "プレビュー用SRVDescriptorのコピー処理に失敗しました。");

    const auto& l_textureID = l_editorManager.FetchVALImGuiTextureID(l_imGuiSRVDescriptorIndex);

    // ビューポート画像(直前のItem)の右下から、余白を空けた位置にプレビューを置く
    // 大きさは、プレビュー用のテクスチャと同じ(ビューポートの幅の4分の1)
    const auto& l_previewClientSize = l_renderer.GetREFPreviewClientSize();
    const auto& l_viewportMAX       = ImGui::GetItemRectMax             ();

    const ImVec2 l_previewMAX = { l_viewportMAX.x - k_cameraPreviewMargin, l_viewportMAX.y - k_cameraPreviewMargin };
    const ImVec2 l_previewMIN = { l_previewMAX.x - static_cast<float>(l_previewClientSize.m_width), l_previewMAX.y - static_cast<float>(l_previewClientSize.m_height) };

    // Item(ボタンなど)にはせず、描画リストへ直接描くため、
    // ビューポート画像の入力(右クリックでのカメラ操作)の判定には影響しない
    auto* l_drawList = ImGui::GetWindowDrawList();

    const auto& l_uvMIN = ImVec2(k_viewportUVMINX, k_viewportUVMINY);
    const auto& l_uvMAX = ImVec2(k_viewportUVMAXX, k_viewportUVMAXY);

    l_drawList->AddImage(l_textureID, l_previewMIN, l_previewMAX, l_uvMIN, l_uvMAX);

    // プレビューの範囲が分かるよう、アクセントカラーの枠線を描く
    const auto l_borderColor = ImGui::ColorConvertFloat4ToU32(Constant::k_imguiAccentColor);

    l_drawList->AddRect(l_previewMIN,
                        l_previewMAX,
                        l_borderColor,
                        k_cameraPreviewBorderRounding,
                        ImDrawFlags_None,
                        k_cameraPreviewBorderThickness);
}

void FWK::Editor::ViewportEditorWindow::RequestCameraPreview(const EditorManager& a_editorManager, const ImVec2& a_viewportSize) const
{
    // 最後に選択したGameObjectを取得する
    // 何も選択していなければ、プレビューは依頼しない(右下に何も描かない)
    const auto& l_gameObjectSelectionState = a_editorManager.GetREFGameObjectSelectionState          ();
    const auto& l_selectedGameObject       = l_gameObjectSelectionState.FindVALLastSelectedGameObject().lock();

    if (!l_selectedGameObject) { return; }

    // 選択したGameObjectがカメラコンポーネントを持っていなければ、プレビューは依頼しない
    const auto& l_componentContainer = l_selectedGameObject->GetREFComponentContainer                     ();
    const auto& l_cameraComponent    = l_componentContainer.FindUniqueComponent<GameObjectCameraComponent>().lock();

    if (!l_cameraComponent) { return; }

    const auto& l_camera       = l_cameraComponent->GetREFCamera();
    const auto& l_cbCameraPass = l_camera.GetREFCBCameraPass    ();

    // プレビューの縦横比はカメラに合わせる
    // カメラのアスペクト比が無効なら、サイズを決められないため依頼しない
    const float l_aspectRatio = l_camera.GetVALAspectRatio();

    if (l_aspectRatio <= Constant::k_cameraInvalidAspectRatio) { return; }

    // プレビューの幅はビューポートの幅のk_cameraPreviewWidthRatio倍(4分の1)
    // 高さはカメラの縦横比から求める
    const auto l_previewWidth  = static_cast<UINT>(a_viewportSize.x * k_cameraPreviewWidthRatio);
    const auto l_previewHeight = static_cast<UINT>(static_cast<float>(l_previewWidth) / l_aspectRatio);

    // 0サイズのテクスチャは作れないため、小さすぎる場合は依頼しない
    if (!Utility::IsValidTextureSize(l_previewWidth, l_previewHeight)) { return; }

    const Struct::WindowClientSize l_previewClientSize = { l_previewWidth, l_previewHeight };

    auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance ();
    auto& l_renderer        = l_graphicsManager.GetMutableREFRenderer();

    l_renderer.RequestPreviewRenderView(l_cbCameraPass, l_previewClientSize);
}

void FWK::Editor::ViewportEditorWindow::RegisterDebugCamera() const
{
    FWK_ASSERT_RETURN_IF(!m_editorCamera, "エディターカメラが無効です。");

    auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance     ();
    auto& l_renderer        = l_graphicsManager.GetMutableREFRenderer    ();
    auto& l_mainRenderView  = l_renderer.GetMutableREFMainRenderView     ();
    auto& l_cameraContext   = l_mainRenderView.GetMutableREFCameraContext();

    // エディターカメラをデバッグカメラとして登録する
    // 描画にデバッグカメラとシーンカメラのどちらを使うかは、CameraContextがモードに合わせて決める
    const auto& l_camera = m_editorCamera->GetREFCamera();

    l_cameraContext.SetDebugCamera(l_camera.GetREFCBCameraPass());
}

void FWK::Editor::ViewportEditorWindow::ReleaseViewportTextureDescriptors()
{
    auto& l_editorManager = EditorManager::GetInstance();

    // 確保済みのImGui用SRVDescriptorIndexをすべてPoolへ返却する
    // 無効値(確保に失敗したIndex)はReleaseImGuiSRVDescriptorIndex内部で無視される
    for (const auto& l_srvDescriptorIndex : m_imGuiSRVDescriptorIndexList)
    {
        l_editorManager.ReleaseImGuiSRVDescriptorIndex(l_srvDescriptorIndex);
    }

    // プレビュー用のImGui用SRVDescriptorIndexも、同じように返却する
    for (const auto& l_srvDescriptorIndex : m_previewImGuiSRVDescriptorIndexList)
    {
        l_editorManager.ReleaseImGuiSRVDescriptorIndex(l_srvDescriptorIndex);
    }

    // 返却したIndexを参照しないようリストも空にする
    m_imGuiSRVDescriptorIndexList.clear       ();
    m_previewImGuiSRVDescriptorIndexList.clear();
}