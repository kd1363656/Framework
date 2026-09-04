#pragma once

namespace FWK
{
	class RotationComponentConstantMouseMode : public RotationComponentConstantModeBase
	{
	public:

		 RotationComponentConstantMouseMode() = default;
		~RotationComponentConstantMouseMode() = default;

		void INIT() override;

		void Update() override;

		void EditInspector() override;

		void Deserialize(const nlohmann::json& a_rootJson) override;

		nlohmann::json Serialize() const override;

	private:

		RotationComponentConstantMouseModeInspector m_inspector = {};

		Converter::RotationComponentConstantMouseModeJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(RotationComponentConstantMouseMode, RotationComponentConstantModeBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RotatationComponentModeUniqueFactory, FWK::RotationComponentConstantMouseMode)