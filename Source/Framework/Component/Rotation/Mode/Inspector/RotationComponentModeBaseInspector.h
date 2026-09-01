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

		void EditInspector(RotationComponentModeBase& a_rotationComponentModeBase);

	private:

		static constexpr std::string_view k_rotationApplyAxisCheckboxLabel = "回転軸適用チェックボックス";
	};
}