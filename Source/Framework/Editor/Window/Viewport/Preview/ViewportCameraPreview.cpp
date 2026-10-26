#include "ViewportCameraPreview.h"

FWK::Editor::ViewportCameraPreview::ViewportCameraPreview() :
    m_previewCamera(),

    m_panelSize(Constant::k_viewportCameraPreviewDefaultPanelSize),
    m_panelMin (),
    m_panelMax (),

    m_copiedCaptureTextureVersion(Constant::k_viewportCameraPreviewNotCopiedTextureVersion),

    m_imGuiSRVDescriptorIndex(Graphics::DescriptorHeap::k_invalidDescriptorIndex),

    m_isPreviewCameraSetup(false),
    m_isPanelVisible      (false)
{}
FWK::Editor::ViewportCameraPreview::~ViewportCameraPreview()
{
    // ImGui用に確保したSRVの番号を返す(ViewportEditorWindowのデストラクタと同じ扱い)
    if (m_imGuiSRVDescriptorIndex == Graphics::DescriptorHeap::k_invalidDescriptorIndex) { return; }

    auto& l_editorManager = EditorManager::GetInstance();

    l_editorManager.ReleaseImGuiSRVDescriptorIndex(m_imGuiSRVDescriptorIndex);
}

void FWK::Editor::ViewportCameraPreview::Draw(const std::weak_ptr<GameObjectCameraComponent>& a_cameraComponent, const Struct::ViewportScreenRect& a_viewportRect)
{
    m_isPanelVisible = false;

    const auto& l_cameraComponent = a_cameraComponent.lock();

    if (!l_cameraComponent) { return; }

    // ビューポートに収まる大きさにする(ビューポートが小さすぎて最小の大きさも入らなければ表示しない)
    const ImVec2 l_maxPanelSize = { a_viewportRect.m_size.x - k_panelMargin * Constant::k_viewportCameraPreviewMarginSideCount,
                                    a_viewportRect.m_size.y - k_panelMargin * Constant::k_viewportCameraPreviewMarginSideCount };

    if (l_maxPanelSize.x < k_minPanelSize.x ||
        l_maxPanelSize.y < k_minPanelSize.y)
    {
        return;
    }

    m_panelSize.x = std::clamp(m_panelSize.x, k_minPanelSize.x, l_maxPanelSize.x);
    m_panelSize.y = std::clamp(m_panelSize.y, k_minPanelSize.y, l_maxPanelSize.y);

    // 右下の角を基準に置く(左上の角をドラッグして大きさを変える)
    m_panelMax = { a_viewportRect.m_min.x + a_viewportRect.m_size.x - k_panelMargin,
                   a_viewportRect.m_min.y + a_viewportRect.m_size.y - k_panelMargin };
    m_panelMin = { m_panelMax.x - m_panelSize.x,
                   m_panelMax.y - m_panelSize.y };

    // 表示枠の縦横比でカメラを作り、次のフレームにRendererへ描いてもらう
    // (Rendererはこのウィンドウより先に描画を終えているため、依頼が反映されるのは次のフレーム)
    UpdatePreviewCamera(*l_cameraComponent, m_panelSize.x / m_panelSize.y);

    auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance      ();
    auto& l_renderer          = l_graphicsManager.GetMutableREFRenderer     ();
    auto& l_renderViewCapture = l_renderer.GetMutableREFRenderViewCapture();

    l_renderViewCapture.Request(m_previewCamera.GetREFCBCameraPass());

    auto* const l_drawList = ImGui::GetWindowDrawList();

    l_drawList->AddRectFilled(m_panelMin, m_panelMax, k_panelBackgroundColor);

    // このフレームで描かれた絵があれば表示する(選んだ直後のフレームは、まだ描かれていない)
    if (ImTextureID l_textureID = {};
        l_renderViewCapture.GetVALIsCapturedThisFrame() &&
        FetchVALCaptureTextureID(l_textureID))
    {
        // 描画結果はFinalColorと同じ大きさ(ウィンドウの大きさ)だが、表示枠の縦横比で投影しているので、枠いっぱいに引き伸ばすと元の形に戻る
        l_drawList->AddImage(l_textureID, m_panelMin, m_panelMax);
    }
    else
    {
        const ImVec2 l_waitingTextPosition = { m_panelMin.x + k_titlePadding.x,
                                               m_panelMax.y - ImGui::GetTextLineHeight() - k_titlePadding.y };

        l_drawList->AddText(l_waitingTextPosition, k_titleTextColor, k_waitingLabel.data());
    }

    // 左上にカメラを持つGameObjectの名前を出す
    if (const auto& l_owner = l_cameraComponent->GetREFOwner().lock();
        l_owner)
    {
        const auto& l_name     = l_owner->GetREFName();
        const auto& l_textSize = ImGui::CalcTextSize(l_name.c_str());

        const ImVec2 l_titleMax      = { m_panelMin.x + l_textSize.x + k_titlePadding.x * Constant::k_viewportCameraPreviewMarginSideCount,
                                         m_panelMin.y + l_textSize.y + k_titlePadding.y * Constant::k_viewportCameraPreviewMarginSideCount };
        const ImVec2 l_titlePosition = { m_panelMin.x + k_titlePadding.x,
                                         m_panelMin.y + k_titlePadding.y };

        l_drawList->AddRectFilled(m_panelMin,      l_titleMax,       k_titleBackgroundColor);
        l_drawList->AddText      (l_titlePosition, k_titleTextColor, l_name.c_str());
    }

    l_drawList->AddRect(m_panelMin,
                        m_panelMax,
                        k_panelBorderColor,
                        k_panelRounding,
                        ImDrawFlags_None,
                        k_panelBorderThickness);

    DrawResizeHandle(a_viewportRect.m_size);

    m_isPanelVisible = true;
}

bool FWK::Editor::ViewportCameraPreview::FetchVALIsMouseOverPanel() const
{
    if (!m_isPanelVisible) { return false; }

    return ImGui::IsMouseHoveringRect(m_panelMin, m_panelMax);
}

void FWK::Editor::ViewportCameraPreview::UpdatePreviewCamera(const GameObjectCameraComponent& a_cameraComponent, const float a_aspectRatio)
{
    // 選択中のカメラと同じ位置、向き、視野角で、縦横比だけを表示枠に合わせる
    const auto& l_sourceCamera = a_cameraComponent.GetREFCamera();

    if (!m_isPreviewCameraSetup)
    {
        m_previewCamera.SetCameraMatrix(l_sourceCamera.GetREFCameraMatrix());
        m_previewCamera.SetAspectRatio (a_aspectRatio);
        m_previewCamera.SetFovYDegree  (l_sourceCamera.GetVALFovYDegree());
        m_previewCamera.SetFarClip     (l_sourceCamera.GetVALFarClip   ());
        m_previewCamera.SetNearClip    (l_sourceCamera.GetVALNearClip  ());

        // Rendererへは登録しない(登録するとメインカメラが差し替わってしまうため)
        m_previewCamera.SetupDetached();

        m_isPreviewCameraSetup = true;

        return;
    }

    m_previewCamera.ApplyProjectionMatrix(a_aspectRatio,
                                          l_sourceCamera.GetVALFovYDegree(),
                                          l_sourceCamera.GetVALFarClip   (),
                                          l_sourceCamera.GetVALNearClip  ());

    m_previewCamera.ApplyCameraMatrix(l_sourceCamera.GetREFCameraMatrix());
}

void FWK::Editor::ViewportCameraPreview::DrawResizeHandle(const ImVec2& a_viewportSize)
{
    ImGui::SetCursorScreenPos(m_panelMin);

    // ImGui::InvisibleButton : 見えないボタン。クリックやドラッグを受け取る範囲だけを作る
    ImGui::InvisibleButton(k_resizeHandleLabel.data(), k_resizeHandleSize);

    const bool l_isHovered = ImGui::IsItemHovered();
    const bool l_isActive  = ImGui::IsItemActive ();

    if (l_isHovered ||
        l_isActive)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
    }

    if (l_isActive)
    {
        // 右下の角は動かないので、左上の角を左上へ動かすほど大きくなる
        const auto& l_io = ImGui::GetIO();

        m_panelSize.x = std::clamp(m_panelSize.x - l_io.MouseDelta.x, k_minPanelSize.x, a_viewportSize.x);
        m_panelSize.y = std::clamp(m_panelSize.y - l_io.MouseDelta.y, k_minPanelSize.y, a_viewportSize.y);
    }

    // 左上の角に三角形の目印を描く
    auto* const l_drawList = ImGui::GetWindowDrawList();

    l_drawList->AddTriangleFilled(m_panelMin,
                                  { m_panelMin.x + k_resizeHandleSize.x, m_panelMin.y },
                                  { m_panelMin.x,                        m_panelMin.y + k_resizeHandleSize.y },
                                  k_resizeHandleColor);
}

bool FWK::Editor::ViewportCameraPreview::FetchVALCaptureTextureID(ImTextureID& a_outTextureID)
{
    const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance  ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer        ();
    const auto& l_renderViewCapture = l_renderer.GetREFRenderViewCapture      ();
    const auto& l_captureTexture    = l_renderViewCapture.GetREFCaptureTexture();

    if (!l_captureTexture.IsCreated()) { return false; }

    auto& l_editorManager = EditorManager::GetInstance();

    if (m_imGuiSRVDescriptorIndex == Graphics::DescriptorHeap::k_invalidDescriptorIndex)
    {
        m_imGuiSRVDescriptorIndex = l_editorManager.AllocateImGuiSRVDescriptorIndex();

        if (m_imGuiSRVDescriptorIndex == Graphics::DescriptorHeap::k_invalidDescriptorIndex) { return false; }
    }

    // Textureが作り直された(初めて作られた、ウィンドウのリサイズで大きさが変わった)ときだけ、
    // 描画用のSRVをImGui用のDescriptorHeapへコピーし直す(ViewportのSetupViewportTextureDescriptorsと同じ仕組み)
    if (const auto l_captureTextureVersion = l_renderViewCapture.GetVALCaptureTextureVersion();
        l_captureTextureVersion != m_copiedCaptureTextureVersion)
    {
        const auto& l_resourceContext         = l_graphicsManager.GetREFResourceContext        ();
        const auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetREFCBVSRVUAVDescriptorPool();

        if (!l_editorManager.CopyGraphicsSRVDescriptor(l_cbvSRVUAVDescriptorPool, l_captureTexture.GetVALSRVDescriptorIndex(), m_imGuiSRVDescriptorIndex)) { return false; }

        m_copiedCaptureTextureVersion = l_captureTextureVersion;
    }

    a_outTextureID = l_editorManager.FetchVALImGuiTextureID(m_imGuiSRVDescriptorIndex);

    return true;
}
