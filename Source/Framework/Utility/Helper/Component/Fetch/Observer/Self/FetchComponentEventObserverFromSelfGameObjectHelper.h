#pragma once

namespace FWK::Utility
{
	class FetchComponentEventObserverFromSelfGameObjectHelper final
	{
	public:

		 FetchComponentEventObserverFromSelfGameObjectHelper() = default;
		~FetchComponentEventObserverFromSelfGameObjectHelper() = default;

		void PostDeserialize(const GameObject& a_self);

		const auto& GetREFFetchedComponentEventObserver() const { return m_fetchedComponentEventObserver; }

	private:

		std::weak_ptr<Observer<Enum::ComponentEvent>> m_fetchedComponentEventObserver = {};
	};
}