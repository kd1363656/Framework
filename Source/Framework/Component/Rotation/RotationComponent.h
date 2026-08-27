#pragma once

namespace FWK
{
	class RotationComponent final : public ComponentBase
	{
	public:

		 RotationComponent() = default;
		~RotationComponent() = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;

		void Update() override;

		void EditInspector() override;

		nlohmann::json SerializePrefab() override;

	private:
		
		std::shared_ptr<RotationComponentModeBase> m_rotationMode = nullptr;

		RotationComponentInspector m_inspector = {};

		Converter::RotationComponentJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(RotationComponent, ComponentBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::RotationComponent)