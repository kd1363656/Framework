#pragma once

namespace FWK
{
	class GameObject final : public std::enable_shared_from_this<GameObject>
	{
	public:

		 GameObject() = default;
		~GameObject() = default;

		void ConfirmMatrix();

		const auto& GetREFParent() const { return m_parent; }

		std::weak_ptr<TransformComponent> GetVALREFTransformComponent() const { return m_transformComponent; }

	private:

		std::vector<std::shared_ptr<ComponentBase>> m_componentBase = {};

		std::shared_ptr<TransformComponent> m_transformComponent = std::make_shared<TransformComponent>();

		std::weak_ptr<GameObject> m_parent = {};

		UUID m_uuid = GUID_NULL;

		std::string m_selfName   = {};
		std::string m_prefabName = {};

		bool m_isDestroyed = false;
	};
}