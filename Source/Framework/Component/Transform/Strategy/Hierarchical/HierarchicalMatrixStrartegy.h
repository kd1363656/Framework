#pragma once

namespace FWK
{
	class HierarchicalMatrixStrartegy final : public MatrixStrategyBase
	{
	public:

		 HierarchicalMatrixStrartegy()          = default;
		~HierarchicalMatrixStrartegy() override = default;

		void Execute(TransformComponent& a_transformComponent) override;

		FWK_DEFINE_TYPE_INFO(HierarchicalMatrixStrartegy, MatrixStrategyBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::MatrixStrategyUniqueFactory, FWK::HierarchicalMatrixStrartegy)