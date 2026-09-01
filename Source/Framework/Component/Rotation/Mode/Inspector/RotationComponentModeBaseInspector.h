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

		void EditInspector(const RotationComponentModeBase& a_rotationComponentModeBase);
	};
}