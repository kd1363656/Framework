#include "ViewportDebugCamera.h"

void FWK::Editor::ViewportDebugCamera::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::ViewportDebugCamera::Update(const float a_aspectRatio, const bool a_isViewportHovered)
{
    // リリースビルドや、無効にしている間は動かさない
    if (!FetchVALIsActive())
    {
        m_isControlling = false;

        return;
    }

    // 縦横比が決まっていない(ビューポートがまだ一度も描かれていない)うちはカメラを作れない
    if (a_aspectRatio <= Constant::k_cameraInvalidAspectRatio) { return; }

    UpdateInput       (a_isViewportHovered);
    UpdateCameraMatrix(a_aspectRatio);
}

void FWK::Editor::ViewportDebugCamera::ApplyRenderView(Graphics::CameraPassDrawRequest& a_cameraPassDrawRequest) const
{
    if (!FetchVALIsActive() ||
        !m_isCameraSetup)
    {
        ReleaseRenderView(a_cameraPassDrawRequest);

        return;
    }

    // 描画はこのカメラから行い、カリングはメインカメラ(CameraComponentが登録したカメラ)で行う
    a_cameraPassDrawRequest.SetOverrideSourceConstantBuffer(m_camera.GetREFCBCameraPass());
    a_cameraPassDrawRequest.SetCullingSourceConstantBuffer (a_cameraPassDrawRequest.GetREFSourceConstantBuffer());
    a_cameraPassDrawRequest.SetIsCullingDebugVisualize     (m_isCullingDebugVisualize);
}
void FWK::Editor::ViewportDebugCamera::ReleaseRenderView(Graphics::CameraPassDrawRequest& a_cameraPassDrawRequest) const
{
    // 差し替えを外すと、描画もカリングもメインカメラに戻る
    a_cameraPassDrawRequest.SetOverrideSourceConstantBuffer({});
    a_cameraPassDrawRequest.SetCullingSourceConstantBuffer ({});
    a_cameraPassDrawRequest.SetIsCullingDebugVisualize     (false);
}

void FWK::Editor::ViewportDebugCamera::DrawMainCameraFrustum(ImDrawList&                       a_drawList,
                                                             const Struct::ViewportScreenRect& a_viewportRect,
                                                             const Struct::CBCameraPass&       a_drawCameraConstantBuffer,
                                                             const Struct::CBCameraPass&       a_mainCameraConstantBuffer) const
{
    if (!FetchVALIsActive() ||
        !m_isDrawMainCameraFrustum)
    {
        return;
    }

    // メインカメラのビュープロジェクション行列の逆行列を使うと、
    // 画面の四隅(手前と奥の2組 = 8点)をワールド座標へ戻せる。その8点を結ぶと視錐台になる
    const auto& l_inverseViewProjectionMatrix = a_mainCameraConstantBuffer.m_viewProjectionMatrix.Invert();

    std::array<TypeAlias::Math::Vector3, k_frustumCornerCount> l_worldCornerList = {};

    for (std::size_t l_cornerIndex = 0ULL; l_cornerIndex < k_frustumCornerCount; ++l_cornerIndex)
    {
        const TypeAlias::Math::Vector3 l_ndcCorner = k_ndcFrustumCornerList[l_cornerIndex];

        // Vector3::TransformはXMVector3TransformCoordと同じで、行列を掛けた後にwで割るため、NDCからワールドへ戻せる
        l_worldCornerList[l_cornerIndex] = TypeAlias::Math::Vector3::Transform(l_ndcCorner, l_inverseViewProjectionMatrix);
    }

    for (const auto& [l_startIndex, l_endIndex] : k_frustumEdgeList)
    {
        DrawClippedLine(a_drawList,
                        a_viewportRect,
                        a_drawCameraConstantBuffer.m_viewProjectionMatrix,
                        l_worldCornerList[l_startIndex],
                        l_worldCornerList[l_endIndex]);
    }
}

void FWK::Editor::ViewportDebugCamera::EditInspector()
{
    m_editInspector.EditInspector(*this);
}

nlohmann::json FWK::Editor::ViewportDebugCamera::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::ViewportDebugCamera::MoveToCameraMatrix(const TypeAlias::Math::Matrix& a_cameraMatrix)
{
    m_position = a_cameraMatrix.Translation();

    // カメラ行列の3行目がカメラの前方向(左手系の+Z)
    // 前方向 = (sin(ヨー) × cos(ピッチ), -sin(ピッチ), cos(ヨー) × cos(ピッチ)) なので、そこから角度を逆算する
    auto l_forward = TypeAlias::Math::Vector3{ a_cameraMatrix._31, a_cameraMatrix._32, a_cameraMatrix._33 };

    l_forward.Normalize();

    m_yawDegree   = DirectX::XMConvertToDegrees(std::atan2(l_forward.x, l_forward.z));
    m_pitchDegree = std::clamp(DirectX::XMConvertToDegrees(std::asin(-l_forward.y)), k_minPitchDegree, k_maxPitchDegree);
}

bool FWK::Editor::ViewportDebugCamera::FetchVALIsActive() const
{
    return Constant::k_isViewportDebugCameraAvailable &&
           m_isEnabled;
}

void FWK::Editor::ViewportDebugCamera::UpdateInput(const bool a_isViewportHovered)
{
    const auto& l_io = ImGui::GetIO();

    // ビューポートの上で右ボタンを押したら操作を始め、離したら終わる
    // (押している間はマウスがビューポートの外へ出ても操作を続ける)
    if (a_isViewportHovered &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        m_isControlling = true;
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        m_isControlling = false;
    }

    const auto& l_rotation = CalculateRotation();

    // 左手系では+Zが前、+Xが右、+Yが上
    // カメラの回転を掛けると、カメラから見た前、右の向きになる
    const auto& l_forward = TypeAlias::Math::Vector3::Transform(TypeAlias::Math::Vector3::UnitZ, l_rotation);
    const auto& l_right   = TypeAlias::Math::Vector3::Transform(TypeAlias::Math::Vector3::UnitX, l_rotation);

    // ホイールは操作中でなくても、ビューポートの上なら前後に動かす
    if (a_isViewportHovered)
    {
        m_position += l_forward * (l_io.MouseWheel * m_moveSpeed * k_wheelMoveSecond);
    }

    if (!m_isControlling) { return; }

    // マウスが動いた量(ピクセル)だけ向きを変える(右へ動かすと右を向き、下へ動かすと下を向く)
    m_yawDegree   += l_io.MouseDelta.x * m_rotationSpeedDegree;
    m_pitchDegree += l_io.MouseDelta.y * m_rotationSpeedDegree;

    // 真上、真下を向くと前方向が決まらなくなるため、少し手前で止める
    m_pitchDegree = std::clamp(m_pitchDegree, k_minPitchDegree, k_maxPitchDegree);

    // 押されているキーの向きを足し合わせる
    auto l_moveDirection = TypeAlias::Math::Vector3::Zero;

    if (ImGui::IsKeyDown(ImGuiKey_W)) { l_moveDirection += l_forward * k_positiveAxisSign; }
    if (ImGui::IsKeyDown(ImGuiKey_S)) { l_moveDirection += l_forward * k_negativeAxisSign; }
    if (ImGui::IsKeyDown(ImGuiKey_D)) { l_moveDirection += l_right   * k_positiveAxisSign; }
    if (ImGui::IsKeyDown(ImGuiKey_A)) { l_moveDirection += l_right   * k_negativeAxisSign; }
    if (ImGui::IsKeyDown(ImGuiKey_E)) { l_moveDirection += TypeAlias::Math::Vector3::UnitY * k_positiveAxisSign; }
    if (ImGui::IsKeyDown(ImGuiKey_Q)) { l_moveDirection += TypeAlias::Math::Vector3::UnitY * k_negativeAxisSign; }

    if (l_moveDirection == TypeAlias::Math::Vector3::Zero) { return; }

    // 斜めに動いても速さが変わらないように、向きの長さを1にする
    l_moveDirection.Normalize();

    const float l_speedMultiplier = l_io.KeyShift ? k_fastMoveMultiplier : k_normalMultiplier;

    // 速さ(メートル/秒) × 前のフレームからの経過時間(秒) = このフレームで動く距離
    m_position += l_moveDirection * (m_moveSpeed * l_speedMultiplier * l_io.DeltaTime);
}
void FWK::Editor::ViewportDebugCamera::UpdateCameraMatrix(const float a_aspectRatio)
{
    // カメラ行列 = 回転 × 位置(カメラのワールド行列)
    const auto& l_cameraMatrix = TypeAlias::Math::Matrix::CreateFromQuaternion(CalculateRotation()) * TypeAlias::Math::Matrix::CreateTranslation(m_position);

    if (!m_isCameraSetup)
    {
        // Rendererへ登録しないカメラとして作る(登録するとメインカメラが差し替わってしまうため)
        m_camera.SetCameraMatrix(l_cameraMatrix);
        m_camera.SetAspectRatio (a_aspectRatio);
        m_camera.SetupDetached  ();

        m_isCameraSetup = true;

        return;
    }

    if (a_aspectRatio != m_camera.GetVALAspectRatio())
    {
        m_camera.ApplyPerspectiveAspectRatio(a_aspectRatio);
    }

    m_camera.ApplyCameraMatrix(l_cameraMatrix);
}

FWK::TypeAlias::Math::Quaternion FWK::Editor::ViewportDebugCamera::CalculateRotation() const
{
    // ヨー(左右の向き、Y軸回り)とピッチ(上下の向き、X軸回り)から回転を作る(ロールは使わない)
    return TypeAlias::Math::Quaternion::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(m_yawDegree), DirectX::XMConvertToRadians(m_pitchDegree), k_noRollRadian);
}

void FWK::Editor::ViewportDebugCamera::DrawClippedLine(ImDrawList&                       a_drawList,
                                                       const Struct::ViewportScreenRect& a_viewportRect,
                                                       const TypeAlias::Math::Matrix&    a_viewProjectionMatrix,
                                                       const TypeAlias::Math::Vector3&   a_startPosition,
                                                       const TypeAlias::Math::Vector3&   a_endPosition) const
{
    auto l_startClip = Utility::TransformViewportWorldToClip(a_startPosition, a_viewProjectionMatrix);
    auto l_endClip   = Utility::TransformViewportWorldToClip(a_endPosition,   a_viewProjectionMatrix);

    const bool l_isStartBehind = l_startClip.w <= Constant::k_viewportProjectionMinClipW;
    const bool l_isEndBehind   = l_endClip.w   <= Constant::k_viewportProjectionMinClipW;

    // 両端ともカメラの後ろにあれば、線は見えない
    if (l_isStartBehind &&
        l_isEndBehind)
    {
        return;
    }

    // 片方だけ後ろにある線は、wがちょうど最小値になる所で切る
    // (カメラの後ろの点をそのまま画面へ投影すると、反対側へ飛んだおかしな線になるため)
    if (l_isStartBehind ||
        l_isEndBehind)
    {
        const float l_ratio      = (Constant::k_viewportProjectionMinClipW - l_startClip.w) / (l_endClip.w - l_startClip.w);
        const auto& l_clipOnEdge = TypeAlias::Math::Vector4::Lerp(l_startClip, l_endClip, l_ratio);

        if (l_isStartBehind) { l_startClip = l_clipOnEdge; }
        else                 { l_endClip   = l_clipOnEdge; }
    }

    const auto& l_startScreen = Utility::ConvertViewportClipToScreen(l_startClip, a_viewportRect);
    const auto& l_endScreen   = Utility::ConvertViewportClipToScreen(l_endClip,   a_viewportRect);

    a_drawList.AddLine(l_startScreen, l_endScreen, k_frustumLineColor, k_frustumLineThickness);
}
