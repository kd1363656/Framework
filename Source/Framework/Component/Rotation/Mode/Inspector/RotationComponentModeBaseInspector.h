#pragma once

namespace FWK
{
	class RotationComponentModeBase;
}

namespace FWK
{
	class RotationComponentModeBaseInspector final
	{
	public:

		 RotationComponentModeBaseInspector() = default;
		~RotationComponentModeBaseInspector() = default;

		void EditInspector(RotationComponentModeBase& a_moveComponentModeBase);

	private:

		static constexpr std::string_view k_rotationApplyAxisBitShiftFlagCheckboxLabel = "回転軸適用チェックボックス";
	};
}