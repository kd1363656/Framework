#pragma once

namespace FWK
{
	class RotationComponentConstantMouseMode : public RotationComponentConstantModeBase
	{
	public:

		 RotationComponentConstantMouseMode()          = default;
		~RotationComponentConstantMouseMode() override = default;

		void INIT() override;

		void Update() override;

		void EditInspector() override;

		void Deserialize(const nlohmann::json& a_rootJson) override;

		nlohmann::json Serialize() const override;

		void SetXAxisRotatableRange(const Struct::Range<float>& a_set) { m_xAxisRotatableRange = a_set; }

		const auto& GetREFXAxisRotatableRange() const { return m_xAxisRotatableRange; }

		auto& GetMutableREFXAxisRotatableRange() { return m_xAxisRotatableRange; }

	private:

		RotationComponentConstantMouseModeInspector m_inspector = {};

		Converter::RotationComponentConstantMouseModeJsonConverter m_jsonConverter = {};

		Struct::Range<float> m_xAxisRotatableRange = {};

		FWK_DEFINE_TYPE_INFO(RotationComponentConstantMouseMode, RotationComponentConstantModeBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RotatationComponentModeUniqueFactory, FWK::RotationComponentConstantMouseMode)