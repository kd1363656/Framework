#pragma once

namespace FWK::Utility
{
	class FetchTransformComponentFromSelfGameObjectHelper final
	{
	public:

		 FetchTransformComponentFromSelfGameObjectHelper() = default;
		~FetchTransformComponentFromSelfGameObjectHelper() = default;

		void PostDeserialize(const std::weak_ptr<GameObject>& a_self);

		const auto& GetREFFetchedTransformComponent() const { return m_fetchedTransformComponent; }

	private:

		std::weak_ptr<TransformComponent> m_fetchedTransformComponent = {};
	};
}