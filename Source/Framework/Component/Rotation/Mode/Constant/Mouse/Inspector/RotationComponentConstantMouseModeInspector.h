#pragma once

namespace FWK
{
	class RotationComponentConstantMouseMode;
}

namespace FWK
{
	class RotationComponentConstantMouseModeInspector
	{
	public:

		 RotationComponentConstantMouseModeInspector() = default;
		~RotationComponentConstantMouseModeInspector() = default;

		void EditInspector(RotationComponentConstantMouseMode& a_rotationComponentConstantMouseMode);

	private:

		static constexpr std::string_view k_rotationSpeedLabel = "回転スピード";
	};
}