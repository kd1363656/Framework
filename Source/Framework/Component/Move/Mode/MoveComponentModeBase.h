#pragma once

namespace FWK
{
	class MoveComponentModeBase
	{
	public:

		         MoveComponentModeBase() = default;
		virtual ~MoveComponentModeBase() = default;

	private:

		FWK_DEFINE_TYPE_INFO_ROOT(ModelComponentBase)
	};
}