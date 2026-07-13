#pragma once

namespace FWK
{
	class Scene final
	{
	public:

		struct GameObjectData
		{
			std::shared_ptr<GameObject> m_gameObject = nullptr;

			const GameObject* m_gameObjectAddress = nullptr;
		};

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

		const auto& GetREFGameObjectDataList() const { return m_gameObjectDataList; }

	private:

		std::unordered_set<const GameObject*> m_registeredGameObjectDataSet = {};

		std::list<GameObjectData> m_gameObjectDataList = {};

		std::shared_ptr<Graphics::Camera> m_camera = nullptr;

		std::shared_ptr<Graphics::StaticModel>											        m_groundModel                    = nullptr;
		std::shared_ptr<Graphics::StaticModelStandardPerObjectDrawRequestBase::DrawRequestData> m_groundModelStandardDrawRequest = nullptr;

		Graphics::SkeletalAnimationModelRecord m_skeletalAnimationMoidelRecord = {};

		std::unique_ptr<Physics::PhysicsBodyBase> m_staticBoxBody     = nullptr;
		std::unique_ptr<Physics::PhysicsBodyBase> m_staticSphereBody  = nullptr;
		std::unique_ptr<Physics::PhysicsBodyBase> m_staticCapsuleBody = nullptr;
		std::unique_ptr<Physics::PhysicsBodyBase> m_staticMeshBody    = nullptr;

		std::unique_ptr<Physics::PhysicsCharacterVirtualBase> m_characterVirtual = nullptr;

		Converter::SceneJsonConverter m_jsonConverter = {};

		Graphics::LightSystem m_lightSystem = {};

		float m_characterModelRotationYRadians = 0.0F;

		bool m_wasJumpKeyDown = false;
	};
}