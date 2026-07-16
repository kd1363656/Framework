#pragma once

namespace FWK::Graphics
{
	class Renderer;
}

namespace FWK::Graphics
{
	// レンダーパスの依存関係を調べ実行順序を決める、
	// またリソースの状態遷移が必要なら状態遷移を行う。
	class RenderGraph
	{
	private:

		using DrawRequestPassMap         = std::unordered_map<TypeAlias::StaticTypeID, std::weak_ptr<DrawRequestPassBase>>;
		using ComputeRequestPerObjectMap = std::unordered_map < TypeAlias::StaticTypeID, std::weak_ptr<ComputeRequestPerObjectBase>>;
		using DrawRequestPerObjectMap    = std::unordered_map<TypeAlias::StaticTypeID, std::weak_ptr<DrawRequestPerObjectBase>>;

	public:

		 RenderGraph() = default;
		~RenderGraph() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		void Compile    ();

		void BeginFrame (const ResourceContext& a_resourceContext, Renderer& a_renderer);
		void Execute    (const ResourceContext& a_resourceContext, Renderer& a_renderer);
		void EndFrame   (      Renderer&	    a_renderer) const;
		
		nlohmann::json Serialize() const;

		void AddPass(std::unique_ptr<RenderGraphPassBase>&& a_pass);

		void AddDrawRequestPass        (const std::shared_ptr<DrawRequestPassBase>&         a_drawRequestPass);
		void AddComputeRequestPerObject(const std::shared_ptr<ComputeRequestPerObjectBase>& a_computeRequestPerObject);
		void AddDrawRequestPerObject   (const std::shared_ptr<DrawRequestPerObjectBase>&    a_drawRequestPerObject);

		template <Concept::IsDerivedDrawRequestPassBaseConcept DrawRequestPassType>
		std::weak_ptr<DrawRequestPassType> FindVALDrawRequestPass() const
		{
			const auto l_staticTypeID = DrawRequestPassType::GetREFTypeINFO().k_staticTypeID;

			const auto& l_itr = m_drawRequestPassMap.find(l_staticTypeID);

			if (l_itr == m_drawRequestPassMap.end()) { return {}; }

			const auto l_drawRequestPass = l_itr->second.lock();

			if (!l_drawRequestPass) { return {}; }

			return std::static_pointer_cast<DrawRequestPassType>(l_drawRequestPass);
		}

		template <Concept::IsDerivedComputeRequestPerObjectBaseConcept ComputeRequestPerObjectType>
		std::weak_ptr<ComputeRequestPerObjectType> FindVALComputeRequestPerObject() const 
		{
			const auto l_staticTypeID = ComputeRequestPerObjectType::GetREFTypeINFO().k_staticTypeID;

			const auto& l_itr = m_computeRequestPerObjectMap.find(l_staticTypeID);

			if (l_itr == m_computeRequestPerObjectMap.end()) { return {}; }

			const auto& l_computeRequestPerObject = l_itr->second.lock();

			if (!l_computeRequestPerObject) { return {}; }

			return std::static_pointer_cast<ComputeRequestPerObjectType>(l_computeRequestPerObject);
		}

		template <Concept::IsDerivedDrawRequestPerObjectBaseConcept DrawRequestPerObjectType>
		std::weak_ptr<DrawRequestPerObjectType> FindVALDrawRequestPerObject() const
		{
			const auto l_staticTypeID = DrawRequestPerObjectType::GetREFTypeINFO().k_staticTypeID;

			const auto& l_itr = m_drawRequestPerObjectMap.find(l_staticTypeID);

			if (l_itr == m_drawRequestPerObjectMap.end()) { return {}; }

			const auto l_drawRequestPerObject = l_itr->second.lock();

			if (!l_drawRequestPerObject) { return {}; }

			return std::static_pointer_cast<DrawRequestPerObjectType>(l_drawRequestPerObject);
		}

		const auto& GetREFPassList() const { return m_passList; }

		const auto& GetREFDrawRequestPassList        () const { return m_drawRequestPassList; }
		const auto& GetREFComputeRequestPerObjectList() const { return m_computeRequestPerObjectList; }
		const auto& GetREFDrawRequestPerObjectList   () const { return m_drawRequestPerObjectList; }

	private:

		void BeginBackBuffer(const ResourceContext& a_resourceContext, Renderer& a_renderer) const;

		void RemoveExpiredPassList();

		DrawRequestPassMap         m_drawRequestPassMap         = {};
		ComputeRequestPerObjectMap m_computeRequestPerObjectMap = {};
		DrawRequestPerObjectMap    m_drawRequestPerObjectMap    = {};

		std::vector<std::unique_ptr<RenderGraphPassBase>> m_passList;

		std::vector<std::shared_ptr<DrawRequestPassBase>>         m_drawRequestPassList         = {};
		std::vector<std::shared_ptr<ComputeRequestPerObjectBase>> m_computeRequestPerObjectList = {};
		std::vector<std::shared_ptr<DrawRequestPerObjectBase>>    m_drawRequestPerObjectList    = {};

		RenderGraphResourceClearer      m_resourceClearer      = {};
		RenderGraphResourceTransitioner m_resourceTransitioner = {};
		RenderGraphResourceBinder		m_resourceBinder	   = {};
		RenderGraphPassSorter			m_passSorter		   = {};

		Converter::RenderGraphJsonConverter m_jsonConverter = {};
	};
}