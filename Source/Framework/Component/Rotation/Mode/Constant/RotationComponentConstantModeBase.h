#pragma once

namespace FWK
{
	class RotationComponentConstantModeBase : public RotationComponentModeBase
	{
	public:

		 RotationComponentConstantModeBase()          = default;
		~RotationComponentConstantModeBase() override = default;

		void INIT() override;

		void Deserialize(const nlohmann::json& a_rootJson);

		void EditInspector() override;

		nlohmann::json Serialize() const override;

		void SetRotationSpeed(const float a_set) { m_rotationSpeed = a_set; }

		float GetVALRotationSpeed() const { return m_rotationSpeed; }

	private:

		RotationComponentConstantModeBaseInspector m_inspector = {};

		Converter::RotationComponentConstantModeBaseJsonConverter m_jsonConverter = {};

		float m_rotationSpeed = Constant::k_rotationComponentConstantDefaultRotationSpeed;

		FWK_DEFINE_TYPE_INFO(RotationComponentConstantModeBase, RotationComponentModeBase)
	};
}