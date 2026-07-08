#pragma once

namespace FWK
{
	class GameObject final
	{
	public:

		 GameObject() = default;
		~GameObject() = default;

		void ConfirmMatrix();

	private:

		std::vector<std::shared_ptr<ComponentBase>> m_componentBase = {};

		std::shared_ptr<TransformComponent> m_transformComponent = std::make_shared<TransformComponent>();

		UUID m_uuid = GUID_NULL;

		std::string m_selfName   = {};
		std::string m_prefabName = {};

		bool m_isDestroyed = false;
	};
}