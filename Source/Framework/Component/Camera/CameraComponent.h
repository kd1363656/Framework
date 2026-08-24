#pragma once

namespace FWK
{
	class CameraComponent final : public FWK::ComponentBase
	{
	public:

		 CameraComponent()          = default;
		~CameraComponent() override = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;

		void PostDeserialize() override;

		void EarlyUpdate   () override;
		void PostLateUpdate() override;

		void EditInspector() override;

		nlohmann::json SerializePrefab() override;

		const auto& GetREFCamera() const { return m_camera; }

		auto& GetMutableREFCamera() { return m_camera; }

	private:

		Graphics::Camera m_camera = {};

		CameraComponentInspector m_inspector = {};

		Utility::FetchTransformComponentFromSelfGameObjectHelper m_fetchTransformComponentFromSelfGameObjectHelper = {};

		Converter::CameraComponentIJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(CameraComponent, ComponentBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::CameraComponent)