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

		static constexpr std::string_view k_xAxisRotatable    = "X軸回転可能角度";
		static constexpr std::string_view k_xAxisRotatableMAX = "最大";
		static constexpr std::string_view k_xAxisRotatableMIN = "最少";
	};
}