#pragma once

namespace FWK
{
    class HierarchicalMatrixStrategy final : public MatrixStrategyBase
    {
    public:

         HierarchicalMatrixStrategy()          = default;
        ~HierarchicalMatrixStrategy() override = default;

        void Execute(TransformComponent& a_transformComponent) override;

        FWK_DEFINE_TYPE_INFO(HierarchicalMatrixStrategy, MatrixStrategyBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::MatrixStrategyUniqueFactory, FWK::HierarchicalMatrixStrategy)