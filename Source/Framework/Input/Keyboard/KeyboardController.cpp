#include "KeyboardController.h"

void FWK::KeyboardController::Update()
{
    // DirectXTK12から現在のキーボード状態を1フレームに1回だけ取得する
    // 各GameObjectやComponentからGetState()を呼ばせず
    // このControllerが取得下同じフレームの状態を全使用側で共有する
    m_state = m_keyboard.GetState();

    // 現在状態と前回状態を比較して、
    // このフレームで押されたキーと話されたキーを更新する
    m_stateTracker.Update(m_state);
}

bool FWK::KeyboardController::IsKeyDown(const DirectX::Keyboard::Keys a_key) const
{
    // Noneはキーが割り当てられていない状態として扱う
    if (a_key == DirectX::Keyboard::None) { return false; }

    return m_state.IsKeyDown(a_key);
}

bool FWK::KeyboardController::IsKeyUp(const DirectX::Keyboard::Keys a_key) const
{
    if (a_key == DirectX::Keyboard::None) { return false; }

    return m_state.IsKeyUp(a_key);
}

bool FWK::KeyboardController::IsKeyPressed(const DirectX::Keyboard::Keys a_key) const
{
    if (a_key == DirectX::Keyboard::None) { return false; }

    return m_stateTracker.IsKeyPressed(a_key);
}

bool FWK::KeyboardController::IsKeyReleased(const DirectX::Keyboard::Keys a_key) const
{
    return m_stateTracker.IsKeyReleased(a_key);
}