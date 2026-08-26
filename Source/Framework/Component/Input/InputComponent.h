#pragma once

namespace FWK
{
	class InputComponent final : public ComponentBase
	{
	public:

		 InputComponent()          = default;
		~InputComponent() override = default;

		void INIT() override;

		bool IsAllowMultiple() const override { return true; }

		void EditInspector() override;

	private:

		InputComponentInspector m_inspector = {};

		FWK_DEFINE_TYPE_INFO(InputComponent, ComponentBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::InputComponent)