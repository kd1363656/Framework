#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestStandardLitStorategy final : public StaticModelRegisterDrawRequestStorategyBase
	{
	public:

		 StaticModelRegisterDrawRequestStandardLitStorategy()          = default;
		~StaticModelRegisterDrawRequestStandardLitStorategy() override = default;

		void Execute(StaticModelComponent& a_staticModelComponent) override;
		
		FWK_DEFINE_TYPE_INFO(StaticModelRegisterDrawRequestStandardLitStorategy, StaticModelRegisterDrawRequestStorategyBase)
	};
}