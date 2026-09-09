#include "FetchComponentEventObserverFromSelfGameObjectHelper.h"

void FWK::Utility::FetchComponentEventObserverFromSelfGameObjectHelper::PostDeserialize(const GameObject& a_self)
{
	const auto& l_componentEventObserver = a_self.GetVALComponentEventObserver();

	if (l_componentEventObserver.expired()) 
	{
		FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントイベントオブザーバーの取得に失敗しました。");

		return; 
	}

	m_fetchedComponentEventObserver = l_componentEventObserver;
}