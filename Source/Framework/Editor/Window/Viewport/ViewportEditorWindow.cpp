#include "ViewportEditorWindow.h"
#include "../../../../Application/Application.h"

FWK::Editor::ViewportEditorWindow::ViewportEditorWindow() :
    m_imGuiSRVDescriptorIndexList(),

    m_editorCamera(std::make_unique<EditorCamera>()),

    m_toolbar()
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
    FWK_ASSERT_RETURN_IF(!m_editorCamera, "エディターカメラが無効です。");

    const auto& l_application = Application::GetInstance  ();
    const auto& l_window      = l_application.GetREFWindow();

    m_editorCamera->Setup(l_window.GetVALAspectRatio());

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

    // エディター表示中はエディターカメラを描画カメラとして登録する
    // (エディター非表示時はシーンカメラ側が自身を登録し直す)
    RegisterEditorCamera();

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
    const auto l_viewportTextureID = FetchVALViewportTextureID();

    // 無効なテクスチャなのでreturn
    if (l_viewportTextureID == k_invalidViewportTextureID)
    {
        ImGui::End();

        return;
    }

    // Viewportの表示領域全体に、取得したTextureを描画する
    DrawViewportTexture(l_viewportTextureID, l_viewportSize);

    // Viewport画像上での入力をエディターカメラへ反映する
    UpdateEditorCameraInput();

    ImGui::End();
}

nlohmann::json FWK::Editor::ViewportEditorWindow::Serialize()
{
    return m_jsonConverter.Serialize(*this);
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
    ImGui::Image(a_textureID,
                 a_viewportSize,
                 l_uvMIN,
                 l_uvMAX);
}

void FWK::Editor::ViewportEditorWindow::RegisterEditorCamera() const
{
    FWK_ASSERT_RETURN_IF(!m_editorCamera, "エディターカメラが無効です。");

          auto& l_graphicsManager  = Graphics::GraphicsManager::GetInstance   ();
          auto& l_renderer         = l_graphicsManager.GetMutableREFRenderer  ();
    const auto& l_renderGraph      = l_renderer.GetREFRenderGraph             ();
          auto& l_cascadeShadowMap = l_renderer.GetMutableREFShadowContext    ().GetMutableREFCascadeShadowMap();

    const auto& l_cbCameraPass = m_editorCamera->GetREFCamera().GetREFCBCameraPass();

    if (const auto& l_cameraPassDrawRequest = l_renderGraph.FindVALDrawRequestPass<Graphics::CameraPassDrawRequest>().lock();
        l_cameraPassDrawRequest)
    {
        // 描画用カメラの定数バッファ参照先をエディターカメラへ差し替える
        l_cameraPassDrawRequest->SetSourceConstantBuffer(l_cbCameraPass);
    }

    // Cascade計算に使用するカメラもエディターカメラへ合わせる
    l_cascadeShadowMap.SetCBCameraPass(l_cbCameraPass);
}

void FWK::Editor::ViewportEditorWindow::UpdateEditorCameraInput()
{
    FWK_ASSERT_RETURN_IF(!m_editorCamera, "エディターカメラが無効です。");

    // 直前に描画したViewport画像上で、右クリック中のみカメラを操作する
    if (!ImGui::IsItemHovered() ||
        !ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        return;
    }

    const auto& l_io = ImGui::GetIO();

    // マウスの移動量をYaw/Pitch回転へ変換する
    m_editorCamera->Rotate(l_io.MouseDelta.x * m_editorCamera->GetVALRotateSpeed(),
                           l_io.MouseDelta.y * m_editorCamera->GetVALRotateSpeed());

    // WASD/QEでカメラローカル方向の移動量を組み立てる
    auto l_localDirection = TypeAlias::Math::Vector3::Zero;

    if (ImGui::IsKeyDown(ImGuiKey_W)) { l_localDirection.z += TypeAlias::Math::Vector3::UnitZ.z; }
    if (ImGui::IsKeyDown(ImGuiKey_S)) { l_localDirection.z -= TypeAlias::Math::Vector3::UnitZ.z; }
    if (ImGui::IsKeyDown(ImGuiKey_D)) { l_localDirection.x += TypeAlias::Math::Vector3::UnitX.x; }
    if (ImGui::IsKeyDown(ImGuiKey_A)) { l_localDirection.x -= TypeAlias::Math::Vector3::UnitX.x; }
    if (ImGui::IsKeyDown(ImGuiKey_E)) { l_localDirection.y += TypeAlias::Math::Vector3::UnitY.y; }
    if (ImGui::IsKeyDown(ImGuiKey_Q)) { l_localDirection.y -= TypeAlias::Math::Vector3::UnitY.y; }

    if (l_localDirection != TypeAlias::Math::Vector3::Zero)
    {
        l_localDirection.Normalize();

        m_editorCamera->Move(l_localDirection, l_io.DeltaTime);
    }
}