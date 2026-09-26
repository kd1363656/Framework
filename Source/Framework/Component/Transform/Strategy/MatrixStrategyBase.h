#pragma once

namespace FWK
{
    class MatrixStrategyBase : public StrategyBase<TransformComponent>
    {
    public:

         MatrixStrategyBase()          = default;
        ~MatrixStrategyBase() override = default;

        virtual bool IsUseParentMatrix() { return true; }

        FWK_DEFINE_TYPE_INFO_ROOT(MatrixStrategyBase)
    };
}