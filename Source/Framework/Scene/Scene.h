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

		void EarlyUpdate();
		void Update     ();
		void LateUpdate () const;
		void FixMatrix  ();
		
		nlohmann::json Serialize() const;

		void AddGameObject(const std::shared_ptr<GameObject>& a_gameObject);

		const auto& GetREFGameObjectList() const { return m_gameObjectList; }

	private:

		std::unordered_set<const GameObject*> m_registeredGameObjectSet = {};

		std::list<Struct::GameObjectData> m_gameObjectList = {};

		std::shared_ptr<Graphics::Camera> m_camera = nullptr;

		std::shared_ptr<Graphics::StaticModel>								 m_charaModel					 = nullptr;
		std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData> m_charaModelStandardDrawRequest = nullptr;

		std::shared_ptr<Graphics::StaticModel>								 m_groundModel                    = nullptr;
		std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData> m_groundModelStandardDrawRequest = nullptr;

		std::unique_ptr<Physics::PhysicsBodyBase> m_staticBoxBody     = nullptr;
		std::unique_ptr<Physics::PhysicsBodyBase> m_staticSphereBody  = nullptr;
		std::unique_ptr<Physics::PhysicsBodyBase> m_staticCapsuleBody = nullptr;
		
		Converter::SceneJsonConverter m_jsonConverter = {};

		Graphics::LightSystem m_lightSystem = {};
	};
}