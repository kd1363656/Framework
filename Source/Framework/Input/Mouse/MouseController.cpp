#include "MouseController.h"
#include "../../../Application/Application.h"

void FWK::MouseController::INIT()
{
	const auto& l_application = Application::GetInstance  ();
	const auto& l_window      = l_application.GetREFWindow();

	FWK_ASSERT_RETURN_IF(!l_window.GetREFHWND(), "MouseControllerの初期化に必要なWindowHandleが存在しません。");

	// DirectXTK12のMouseが入力を取得するWindowを設定する
	m_mouse.SetWindow(l_window.GetREFHWND());

	// 初期状態は通常のMouse座標を取得するAbsoluteModeにする
	m_mouse.SetMode(DirectX::Mouse::MODE_ABSOLUTE);

	m_buttonStateTracker.Reset();

	m_currentPosition       = TypeAlias::Math::Vector2::Zero;
	m_previousPosition      = TypeAlias::Math::Vector2::Zero;
	m_movement              = TypeAlias::Math::Vector2::Zero;
	m_isPositionInitialized = false;
}

void FWK::MouseController::Update()
{
	// 現在フレームのMouse状態を一回だけ取得する
	m_state = m_mouse.GetState();

	// 前フレーム比較して
	// PRESSED / HELD / RELEASED / UPを更新する
	m_buttonStateTracker.Update(m_state);

	const auto& l_application = Application::GetInstance  ();
	const auto& l_window      = l_application.GetREFWindow();
	const auto& l_hwnd        = l_window.GetREFHWND       ();

	// ApplicationのMainWindowが操作対象になっている場合だけ
	// MousePositionとMovementを更新する
	if (GetForegroundWindow() == l_hwnd)
	{
		const auto& l_currentPosition = TypeAlias::Math::Vector2{ static_cast<float>(m_state.x), static_cast<float>(m_state.y) };

		// 起動直後やWindowへ戻った直後は
		// 比較対象となる正しいPreviousPositionが存在しないため
		// Movementを計算せず現在座標を基準として登録する
		if (!m_isPositionInitialized)
		{
			m_currentPosition  = l_currentPosition;
			m_previousPosition = l_currentPosition;
			m_movement         = TypeAlias::Math::Vector2::Zero;

			// DirectXTK12のAbsolute座標はWindowMessageによって更新されるため
			// 初期状態のZeroを正しい初期MousePositionとして確定させない
			if (l_currentPosition != TypeAlias::Math::Vector2::Zero) 
			{
				m_isPositionInitialized = true;
			}
		}
		else
		{
			// 現z内フレームのMouse座標を保存する
			m_currentPosition = l_currentPosition;

			// 現在座標 - 前フレーム座標によって
			// このフレームでMouseが移動した量を求める
			m_movement = m_currentPosition - m_previousPosition;

			// 次フレームのMovement計算用に
			// 現在座標をPreviousPositionへ保存する
			m_previousPosition = m_currentPosition;
		}
	}
	else
	{
		// Windowが操作対象ではない間はMovementを発生させない
		m_movement = TypeAlias::Math::Vector2::Zero;

		// Windowへ戻った最初のフレームで
		// 古いPositionとの差分を計算しないようにする
		m_isPositionInitialized = false;
	}

	// このフレームのWheel値はm_stateへ取得済みなので
	// 次フレーム用にDirectXTK12内部の累積値を0へ戻す
	m_mouse.ResetScrollWheelValue();
}

bool FWK::MouseController::IsButtonDown(const MouseButton a_button) const
{
	const auto l_buttonState = FetchVALButtonState(a_button);

	return l_buttonState == DirectX::Mouse::ButtonStateTracker::PRESSED ||
		   l_buttonState == DirectX::Mouse::ButtonStateTracker::HELD;
}
bool FWK::MouseController::IsButtonUp(const MouseButton a_button) const
{
	const auto l_buttonState = FetchVALButtonState(a_button);

	return l_buttonState == DirectX::Mouse::ButtonStateTracker::UP || 
		   l_buttonState == DirectX::Mouse::ButtonStateTracker::RELEASED;
}
bool FWK::MouseController::IsButtonPressed(const MouseButton a_button) const
{	
	return FetchVALButtonState(a_button) == DirectX::Mouse::ButtonStateTracker::PRESSED;
}
bool FWK::MouseController::IsButtonReleased(const MouseButton a_button) const
{
	return FetchVALButtonState(a_button) == DirectX::Mouse::ButtonStateTracker::RELEASED;
}

void FWK::MouseController::SetVisible(const bool a_set)
{
	if (m_mouse.IsVisible() == a_set) { return; }

	m_mouse.SetVisible(a_set);
}

int FWK::MouseController::FetchVALScrollWheelValue() const
{
	return m_state.scrollWheelValue;
}

DirectX::Mouse::ButtonStateTracker::ButtonState FWK::MouseController::FetchVALButtonState(const MouseButton a_button) const
{
	switch (a_button)
	{
		case MouseButton::Left:
		{
			return m_buttonStateTracker.leftButton;
		}
		break;

		case MouseButton::Middle:
		{
			return m_buttonStateTracker.middleButton;
		}
		break;

		case MouseButton::Right:
		{
			return m_buttonStateTracker.rightButton;
		}
		break;

		case MouseButton::XButton1:
		{
			return m_buttonStateTracker.xButton1;
		}
		break;

		case MouseButton::XButton2:
		{
			return m_buttonStateTracker.xButton2;
		}
		break;

		default:
		{
			return DirectX::Mouse::ButtonStateTracker::UP;
		}
		break;
	}
}