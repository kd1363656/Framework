#include "InputManager.h"

void FWK::InputManager::INIT()
{
	// KeyboardControllerはwindowsHandleを必要としないため
	// 現在Init処理はMouseControllerだけに必要
	m_mouseController.INIT();
}

void FWK::InputManager::Update()
{
	// Keyboard / Mouseの現在フレーム状態をそれぞれ一回だけ取得する
	m_keyboardController.Update();
	m_mouseController.Update   ();
}