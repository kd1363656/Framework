#pragma once

namespace FWK
{
	class MatrixStrategyBase : public StrategyBase<FWK::TransformComponent>
	{
	public:

		 MatrixStrategyBase()          = default;
		~MatrixStrategyBase() override = default;

		FWK_DEFINE_TYPE_INFO_ROOT(MatrixStrategyBase)
	};
}