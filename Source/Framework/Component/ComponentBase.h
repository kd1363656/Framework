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

		virtual void INIT() { /*必要に応じてオーバーライドしてください*/ };

		virtual void DeserializePrefab(const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };
		virtual void DeserializeScene (const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };
		
		virtual void PostDeserialize() { /*必要に応じてオーバーライドしてください*/ };

		virtual void EarlyUpdate   () { /*必要に応じてオーバーライドしてください*/ };
		virtual void Update        () { /*必要に応じてオーバーライドしてください*/ };
		virtual void LateUpdate    () { /*必要に応じてオーバーライドしてください*/ };
		virtual void PostLateUpdate() { /*必要に応じてオーバーライドしてください*/ };
		 
		virtual void EditInspector() { /*必要に応じてオーバーライドしてください*/ };

		virtual nlohmann::json SerializePrefab() { return {}; }
		virtual nlohmann::json SerializeScene () { return {}; }

		virtual bool IsAllowMultiple() const { return false; }

		void Enable ();
		void Disable();

		void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

		bool GetVALIsDisable() const { return m_isDisable; }

	protected:

		const auto& GetREFOwner() const { return m_owner; }

	private:

		std::weak_ptr<GameObject> m_owner = {};

		bool m_isDisable = false;

		FWK_DEFINE_TYPE_INFO_ROOT(ComponentBase)
	};
}