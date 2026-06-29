#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK
{
	class ComponentBase
	{
	public:

				 ComponentBase() = default;
		virtual ~ComponentBase() = default;

		virtual bool IsAllowMultiple() const { return false; }

		void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

		void Enable ();
		void Disable();

		bool GetVALIsDisable() const { return m_isDisable; }

		void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

	protected:

		const auto& GetREFOwner() const { return m_owner; }

	private:

		std::weak_ptr<GameObject> m_owner = {};

		bool m_isDisable = false;

		FWK_DEFINE_TYPE_INFO_ROOT(ComponentBase)
	};
}