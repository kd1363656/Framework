#include "InputMouseRightComponentEventNotifyStrategy.h"

void FWK::InputMouseRightComponentEventNotifyStrategy::Execute(InputComponent& a_inputComponent)
{
	const auto& l_gameObject = a_inputComponent.GetREFOwner().lock();

	if (!l_gameObject)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "InputComponentの所有者であるゲームオブジェクトの取得に失敗しましたゲームオブジェクト");

		return;
	}

	const auto& l_componentEventObserver = l_gameObject->GetVALComponentEventObserver().lock();

	if (!l_componentEventObserver)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "コンポーネントイベントオブザーバーが無効となっています。");

		return;
	}

	const auto& l_execution            = a_inputComponent.GetREFExecution    ();
	const auto& l_inputManager         = InputManager::GetInstance           ();
	const auto& l_mouseController      = l_inputManager.GetREFMouseController();
	const auto  l_isPressedMMouseRight = l_mouseController.IsButtonPressed   (MouseController::MouseButton::Right);

	if (l_isPressedMMouseRight)
	{
		l_componentEventObserver->NotifyEvent(l_execution.m_notifyComponentEvent, l_execution.m_notifyEventLane, l_execution.m_notifyFlag);
	}
}