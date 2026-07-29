#pragma once

namespace FWK
{
	class Scene final
	{
	public:

		 Scene() = default;
		~Scene() = default;

		void INIT		();
		void Deserialize(const nlohmann::json& a_rootJson);
		
		// デシリアライズ後のポインタの紐づけなどを行う
		void PostDeserialize() const;

		void EarlyUpdate  ();
		void Update       ();
		void LateUpdate   () const;
		void ConfirmMatrix() const;

		nlohmann::json Serialize() const;

		void AddGameObject(const std::shared_ptr<GameObject>& a_gameObject);

		const auto& GetREFGameObjectList() const { return m_gameObjectList; }

	private:

		std::list<std::shared_ptr<GameObject>> m_gameObjectList = {};

		UUIDRegistry<std::weak_ptr<GameObject>> m_gameObjectUUIDRegistry = {};

		Converter::SceneJsonConverter m_jsonConverter = {};

		Graphics::LightSystem m_lightSystem = {};
	};
}