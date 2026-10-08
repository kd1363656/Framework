#pragma once

namespace FWK
{
    template <typename ArgumentType>
    class StrategyBase
    {
    public:

                 StrategyBase() = default;
        virtual ~StrategyBase() = default;

        virtual void Execute(ArgumentType& a_argument) = 0;
    };
}