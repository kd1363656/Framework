#pragma once

namespace FWK
{
	class MoveComponent;
}

namespace FWK
{
	class MoveComponentInspector final
	{
	public:

		 MoveComponentInspector() = default;
		~MoveComponentInspector() = default;

		void EditInspector(MoveComponent& a_moveComponent);
	};
}