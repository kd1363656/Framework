#pragma once

namespace FWK
{
	class RotationComponentConstantModeBase : public RotationComponentModeBase
	{
	public:

		 RotationComponentConstantModeBase() = default;
		~RotationComponentConstantModeBase() = default;

		void INIT() override;

		void Deserialize(const nlohmann::json& a_rootJson);

		void EditInspector() override;

		nlohmann::json Serialize() override;

		void SetRotationSpeed(const float a_set) { m_rotationSpeed = a_set; }

		float GetVALRotationSpeed() const { return m_rotationSpeed; }

	private:

		float m_rotationSpeed = Constant::k_rotationComponentConstantDefaultRotationSpeed;
	};
}