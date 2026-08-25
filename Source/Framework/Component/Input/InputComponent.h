#pragma once

namespace FWK
{
	class InputComponent final : public ComponentBase
	{
	public:

		 InputComponent();
		~InputComponent();

		bool IsAllowMultiple() const override { return true; }

	private:


		FWK_DEFINE_TYPE_INFO(InputComponent, ComponentBase)
	};
}