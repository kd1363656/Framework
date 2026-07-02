#pragma once

namespace FWK
{
	class Scene final
	{
	public:

		 Scene() = default;
		~Scene() = default;

		void INIT		    ();
		void Deserialize    (const nlohmann::json& a_rootJson);

		// デシリアライズ後のポインタの紐づけなどを行う
		void PostDeserialize() const;

		void EarlyUpdate();
		void Update     ();
		void LateUpdate () const;
		void FixMatrix  ();
		
		nlohmann::json Serialize() const;

	private:

		std::shared_ptr<Graphics::Camera> m_camera = nullptr;

		std::shared_ptr<Graphics::StaticModel>								 m_charaModel					 = nullptr;
		std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData> m_charaModelStandardDrawRequest = nullptr;

		std::shared_ptr<Graphics::StaticModel>								 m_groundModel                    = nullptr;
		std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData> m_groundModelStandardDrawRequest = nullptr;

		Struct::PhysicsBodyHandle m_staticFloorBodyHandle    = {};
		Struct::PhysicsBodyHandle m_staticWallBodyHandle     = {};
		Struct::PhysicsBodyHandle m_dynamicCapsuleBodyHandle = {};

		Converter::SceneJsonConverter m_jsonConverter = {};

		Graphics::LightSystem m_lightSystem = {};
	};
}