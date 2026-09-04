#include "FetchComponentEventObserverFromSelfGameObjectHelper.h"

void FWK::Utility::FetchComponentEventObserverFromSelfGameObjectHelper::PostDeserialize(const std::weak_ptr<GameObject>& a_self)
{
	const auto& l_self = a_self.lock();

	FWK_ASSERT_RETURN_IF(!l_self, "自身のゲームオブジェクトポインタが無効のため、自身のComponentEventObserver取得に失敗しました。");

	const auto& l_componentEventObserver = l_self->GetVALComponentEventObserver();

	if (l_componentEventObserver.expired()) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "コンポーネントイベントオブザーバーの取得に失敗しました。");

		return; 
	}

	m_fetchedComponentEventObserver = l_componentEventObserver;
}