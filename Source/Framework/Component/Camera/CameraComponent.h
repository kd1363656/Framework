#pragma once

namespace FWK
{
	class CameraComponent final : public FWK::ComponentBase
	{
	public:

		 CameraComponent()          = default;
		~CameraComponent() override = default;

	private:

		FWK_DEFINE_TYPE_INFO(CameraComponent, ComponentBase)
	};
}