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

		std::shared_ptr<Graphics::Camera> m_camera = nullptr;

		std::shared_ptr<Graphics::SkeletalAnimationModel>                               m_characterModel                    = nullptr;
		std::shared_ptr<Graphics::SkeletalAnimationPlayer>                              m_characterAnimationPlayer          = nullptr;
		std::shared_ptr<Struct::SkeletalAnimationModelStandardPerObjectDrawRequestData> m_characterModelStandardDrawRequest = nullptr;

		std::shared_ptr<Graphics::StaticModel>								 m_groundModel                    = nullptr;
		std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData> m_groundModelStandardDrawRequest = nullptr;

		std::unique_ptr<Physics::PhysicsBodyBase> m_staticBoxBody     = nullptr;
		std::unique_ptr<Physics::PhysicsBodyBase> m_staticSphereBody  = nullptr;
		std::unique_ptr<Physics::PhysicsBodyBase> m_staticCapsuleBody = nullptr;
		std::unique_ptr<Physics::PhysicsBodyBase> m_staticMeshBody    = nullptr;

		std::unique_ptr<Physics::PhysicsCharacterVirtualBase> m_characterVirtual = nullptr;

		UUIDRegistry<std::weak_ptr<GameObject>> m_gameObjectUUIDRegistry = {};

		Converter::SceneJsonConverter m_jsonConverter = {};

		Graphics::LightSystem m_lightSystem = {};

		float m_characterModelRotationYRadians = 0.0F;

		bool m_wasJumpKeyDown = false;
	};
}