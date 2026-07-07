#pragma once

namespace FWK
{
	template <typename Argument>
	class StrategyBase
	{
	public:
	
		         StrategyBase() = default;
		virtual ~StrategyBase() = default;

		virtual void Execute(Argument& a_argument) = 0;
	};
}