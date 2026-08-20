#include "FetchTransformComponentFromSelfGameObjectHelper.h"

void FWK::Utility::FetchTransformComponentFromSelfGameObjectHelper::PostDeserialize(const std::weak_ptr<GameObject>& a_self)
{
	const auto& l_self = a_self.lock();

	FWK_ASSERT_RETURN_IF(!l_self, "自身のゲームオブジェクトポインタが無効のため、自身のTransformComponent取得に失敗しました。");

	const auto& l_transformComponent = l_self->GetVALTransformComponent();

	// 絶対にあるべきTransformComponentが存在しないためreturn
	FWK_ASSERT_RETURN_IF(l_transformComponent.expired(), "自身のゲームオブジェクトポインタが無効のため、自身のTransformComponent取得に失敗しました。");

	m_fetchedTransformComponent = l_transformComponent;
}