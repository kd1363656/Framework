#pragma once

namespace FWK
{
	class StandaloneMatrixStrategy final : public MatrixStrategyBase
	{
	public:

		 StandaloneMatrixStrategy()          = default;
		~StandaloneMatrixStrategy() override = default;

		void Execute(TransformComponent& a_owner) override;

		FWK_DEFINE_TYPE_INFO(StandaloneMatrixStrategy, MatrixStrategyBase)
	};
}