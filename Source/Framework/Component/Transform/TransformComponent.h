#pragma once

namespace FWK
{
	class TransformComponent final : public ComponentBase
	{
	public:

		 TransformComponent()          = default;
		~TransformComponent() override = default;

	private:

		Struct::Transform m_transform = {};

		FWK_DEFINE_TYPE_INFO(TransformComponent, ComponentBase)
	};
}