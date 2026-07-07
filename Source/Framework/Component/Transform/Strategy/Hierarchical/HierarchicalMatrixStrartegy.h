#pragma once

namespace FWK
{
	class HierarchicalMatrixStrartegy final : public MatrixStrategyBase
	{
	public:

		 HierarchicalMatrixStrartegy()          = default;
		~HierarchicalMatrixStrartegy() override = default;

		void Execute(TransformComponent& a_owner) override;

		FWK_DEFINE_TYPE_INFO(HierarchicalMatrixStrartegy, MatrixStrategyBase)
	};
}