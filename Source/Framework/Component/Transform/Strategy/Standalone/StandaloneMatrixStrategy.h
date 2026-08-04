#pragma once

namespace FWK
{
	class StandaloneMatrixStrategy final : public MatrixStrategyBase
	{
	public:

		 StandaloneMatrixStrategy()          = default;
		~StandaloneMatrixStrategy() override = default;

		void Execute(TransformComponent& a_transformComponent) override;

		FWK_DEFINE_TYPE_INFO(StandaloneMatrixStrategy, MatrixStrategyBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::MatrixStrategyUniqueFactory, FWK::StandaloneMatrixStrategy)