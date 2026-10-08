#include "EditorDebugRenderer.h"

void FWK::Editor::EditorDebugRenderer::CollectDebugDrawCommands(const EditorGameObjectSelectionState& a_gameObjectSelectionState, const bool a_isDrawFrustum)
{
    // 前のフレームの線を消してから、このフレームの線を集め直す
    // 何も集めなければ、キューが空のままになり、このフレームは何も描かれない
    m_editorDebugRendererQueue.ClearFrame();

    // 視錐台の可視化が有効なときだけ、選択中のカメラの視錐台を集める
    if (!a_isDrawFrustum) { return; }

    CollectSelectedCameraFrustum(a_gameObjectSelectionState);
}

void FWK::Editor::EditorDebugRenderer::CollectSelectedCameraFrustum(const EditorGameObjectSelectionState& a_gameObjectSelectionState)
{
    // 選択中のGameObjectを1つずつ調べて、カメラコンポーネントを持つものの視錐台を線として集める
    // 複数選択している場合は、カメラを持つものすべての視錐台を描く
    for (const auto& l_selectedGameObjectWeak : a_gameObjectSelectionState.GetREFSelectedGameObjectList())
    {
        const auto& l_selectedGameObject = l_selectedGameObjectWeak.lock();

        // 無効なGameObject、破棄予定のGameObjectは対象にしない
        if (!l_selectedGameObject ||
            l_selectedGameObject->GetVALIsDestroyed())
        {
            continue;
        }

        // カメラコンポーネントを持っていなければ、次のGameObjectへ
        const auto& l_componentContainer = l_selectedGameObject->GetREFComponentContainer                        ();
        const auto& l_cameraComponent    = l_componentContainer.FindVALUniqueComponent<GameObjectCameraComponent>().lock();

        if (!l_cameraComponent) { continue; }

        const auto& l_camera       = l_cameraComponent->GetREFCamera();
        const auto& l_cbCameraPass = l_camera.GetREFCBCameraPass    ();

        // 視野角から計算したtanHalfFOVは定数バッファが持っている
        // 定数バッファがまだ作られていないカメラは、描けないため次へ
        if (!l_cbCameraPass) { continue; }

        // 奥の面(far)が遠すぎると、線が画面に収まらず見づらい
        // そのため、描画する奥の面までの距離に上限を設ける
        const auto l_farDistance = std::min(l_camera.GetVALFarClip(), Constant::k_editorFrustumDrawMAXDistance);

        // カメラ行列(カメラのワールド行列)を使って、カメラの位置と向きに合わせた視錐台を作る
        m_editorDebugRendererQueue.AddFrustum(l_camera.GetREFCameraMatrix(),
                                              Constant::k_editorFrustumLineColor,
                                              l_cbCameraPass->m_tanHalfFOVX,
                                              l_cbCameraPass->m_tanHalfFOVY,
                                              l_camera.GetVALNearClip(),
                                              l_farDistance);
    }
}