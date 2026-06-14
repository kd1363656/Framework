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

		using DrawRequestPassMap      = std::unordered_map<TypeAlias::StaticTypeID, std::weak_ptr<DrawRequestPassBase>>;
		using DrawRequestPerObjectMap = std::unordered_map<TypeAlias::StaticTypeID, std::weak_ptr<DrawRequestPerObjectBase>>;

	public:

		 RenderGraph() = default;
		~RenderGraph() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		void BeginFrame (const ResourceContext& a_resourceContext, Renderer& a_renderer);
		void Execute    (const ResourceContext& a_resourceContext, Renderer& a_renderer);
		void EndFrame   (      Renderer&	    a_renderer) const;
		
		nlohmann::json Serialize() const;

		void AddPass(std::unique_ptr<RenderGraphPassBase>&& a_pass);

		void AddDrawRequestPass     (const std::shared_ptr<DrawRequestPassBase>&      a_drawRequestPass);
		void AddDrawRequestPerObject(const std::shared_ptr<DrawRequestPerObjectBase>& a_drawRequestPerObject);

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

		const auto& GetREFDrawRequestPassList     () const { return m_drawRequestPassList; }
		const auto& GetREFDrawRequestPerObjectList() const { return m_drawRequestPerObjectList; }

	private:

		void BeginBackBuffer(const ResourceContext& a_resourceContext, Renderer& a_renderer) const;

		void ClearCurrentFrameRenderTargetPassTextureList(const ResourceContext& a_resourceContext, const Renderer& a_renderer)														const;
		bool ClearRenderTargetPassTexture                (const ResourceContext& a_resourceContext, const Renderer& a_renderer, RenderTargetPassTexture& a_renderTargetPassTexture) const;

		bool SetupBackBufferRenderTarget             (const ResourceContext& a_resourceContext, const Renderer&			   a_renderer, const Struct::RenderGraphResourceAccess& a_resourceAccess) const;
		void SetupPassRenderTarget                   (const ResourceContext& a_resourceContext, const RenderGraphPassBase& a_pass,	   const Renderer&						    a_renderer)		  const;
		bool SetupRenderTargetPassTextureRenderTarget(const ResourceContext& a_resourceContext, const Renderer&			   a_renderer, const Struct::RenderGraphResourceAccess& a_resourceAccess) const;

		bool IsWriteRenderTargetAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const;

		void TransitionPassResource                   (const RenderGraphPassBase&				a_pass,		               Renderer&		     a_renderer)									 const;
		bool TransitionBackBufferResource             (const Struct::RenderGraphResourceAccess& a_resourceAccess,          Renderer&		     a_renderer)									 const;
		void TransitionBackBufferResource			  (const DirectCommandList&					a_directCommandList, const D3D12_RESOURCE_STATES a_afterState, Struct::BackBuffer& a_backBuffer) const;
		bool TransitionRenderTargetPassTextureResource(const Struct::RenderGraphResourceAccess& a_resourceAccess,	 const Renderer&			 a_renderer)									 const;
		
		D3D12_RESOURCE_STATES ConvertVALD3D12ResourceState(const Enum::RenderGraphResourceUsage a_usage) const;

		void RemoveExpiredPassList();
		
		DrawRequestPassMap      m_drawRequestPassMap      = {};
		DrawRequestPerObjectMap m_drawRequestPerObjectMap = {};

		std::vector<std::unique_ptr<RenderGraphPassBase>> m_passList = {};

		std::vector<std::shared_ptr<DrawRequestPassBase>>      m_drawRequestPassList      = {};
		std::vector<std::shared_ptr<DrawRequestPerObjectBase>> m_drawRequestPerObjectList = {};

		Converter::RenderGraphJsonConverter m_jsonConverter = {};
	};
}