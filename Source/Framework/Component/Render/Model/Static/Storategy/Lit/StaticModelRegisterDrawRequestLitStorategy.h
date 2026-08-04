#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestLitStorategyBase final : public StaticModelRegisterDrawRequestStorategyBase
	{
	public:

		 StaticModelRegisterDrawRequestLitStorategyBase()          = default;
		~StaticModelRegisterDrawRequestLitStorategyBase() override = default;

		void Execute(StaticModelComponent& a_staticModelComponent) override;
		
		FWK_DEFINE_TYPE_INFO(StaticModelRegisterDrawRequestLitStorategyBase, StaticModelRegisterDrawRequestStorategyBase)
	};
}