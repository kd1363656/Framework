#pragma once

namespace FWK
{
	class RotationComponent;
}

namespace FWK
{
	class RotationComponentInspector final
	{
	public:

		 RotationComponentInspector() = default;
		~RotationComponentInspector() = default;

		void EditInspector(RotationComponent& a_rotationComponent);
	};
}