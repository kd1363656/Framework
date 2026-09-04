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

		nlohmann::json Serialize() override;

	private:


		FWK_DEFINE_TYPE_INFO(RotationComponentConstantMouseMode, RotationComponentConstantModeBase)
	};
}