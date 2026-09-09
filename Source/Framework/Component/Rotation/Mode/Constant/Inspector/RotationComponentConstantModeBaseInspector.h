#pragma once

namespace FWK
{
	class RotationComponentConstantModeBase;
}

namespace FWK
{
	class RotationComponentConstantModeBaseInspector
	{
	public:

		 RotationComponentConstantModeBaseInspector() = default;
		~RotationComponentConstantModeBaseInspector() = default;

		void EditInspector(RotationComponentConstantModeBase& a_moveComponentConstantModeBase);

	private:

		static constexpr std::string_view k_rotationSpeedLabel = "回転スピード";
	};
}