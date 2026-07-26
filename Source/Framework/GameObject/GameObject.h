#pragma once

namespace FWK
{
	class GameObject final : public std::enable_shared_from_this<GameObject>
	{
	public:

		 GameObject() = default;
		~GameObject() = default;

		void PostDeserialize() const;

		void EarlyUpdate  () const;
		void Update       () const;
		void LateUpdate   () const;
		void ConfirmMatrix() const;

		void EditInsepector();

		void AddComponent(const std::shared_ptr<ComponentBase>& a_component);

		void CreateParentChildRelationShip(const std::weak_ptr<GameObject>& a_child);

		void SetParent(const std::weak_ptr<GameObject>& a_set) { m_parent = a_set; }

		template <Concept::IsDerivedComponentBaseConcept ComponentType>
		std::weak_ptr<ComponentType> FindUniqueComponent() const
		{
			const auto l_staticTypeID = ComponentType::GetREFTypeINFO().k_staticTypeID;

			if (auto l_itr = m_uniqueComponentMap.find(l_staticTypeID);
				l_itr != m_uniqueComponentMap.end())
			{
				if (auto l_component = l_itr->second.lock())
				{
					return std::static_pointer_cast<ComponentType>(l_component);
				}
			}
	
			return std::weak_ptr<ComponentType>();
		}

		template <Concept::IsDerivedComponentBaseConcept ComponentType>
		std::vector<std::weak_ptr<ComponentType>> FindMultiComponent() const
		{
			const auto l_staticTypeID = ComponentType::GetREFTypeINFO().k_staticTypeID;

			std::vector<std::weak_ptr<ComponentType>> l_list = {};

			if (auto l_itr = m_multiComponentMap.find(l_staticTypeID);
				l_itr != m_multiComponentMap.end())
			{
				l_list.reserve(l_itr->second.size());

				for (const auto& l_commponent : l_itr->second)
				{
					l_list.emplace_back(l_commponent);
				}
			}
	
			return l_list;
		}

		const auto& GetREFParent() const { return m_parent; }

		std::weak_ptr<TransformComponent> GetVALTransformComponent() const { return m_transformComponent; }

	private:

		std::unordered_map<std::uint32_t, std::weak_ptr<ComponentBase>>				 m_uniqueComponentMap = {};
		std::unordered_map<std::uint32_t, std::vector<std::weak_ptr<ComponentBase>>> m_multiComponentMap  = {};

		Utility::VectorArray<std::weak_ptr<GameObject>>      m_childList     = {};
		Utility::VectorArray<std::shared_ptr<ComponentBase>> m_componentList = {};

		std::shared_ptr<TransformComponent> m_transformComponent = std::make_shared<TransformComponent>();

		std::weak_ptr<GameObject> m_parent = {};

		UUID m_uuid = GUID_NULL;
		
		std::string m_selfName   = {};
		std::string m_prefabName = {};

		bool m_isDestroyed = false;
	};
}