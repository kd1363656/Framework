#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestStandardUnLitStorategy final : public StaticModelRegisterDrawRequestStorategyBase
	{
	public:

		 StaticModelRegisterDrawRequestStandardUnLitStorategy()          = default;
		~StaticModelRegisterDrawRequestStandardUnLitStorategy() override = default;

		void Execute(StaticModelComponent& a_staticModelComponent) override;
		
		FWK_DEFINE_TYPE_INFO(StaticModelRegisterDrawRequestStandardUnLitStorategy, StaticModelRegisterDrawRequestStorategyBase)
	};
}