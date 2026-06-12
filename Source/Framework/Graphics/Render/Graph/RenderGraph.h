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

		using PassMap = std::unordered_map<TypeAlias::StaticTypeID, std::weak_ptr<RenderGraphPassBase>>;

	public:

		 RenderGraph() = default;
		~RenderGraph() = default;

		void Deserialize(const nlohmann::json&  a_rootJson);
		void BeginFrame (const ResourceContext& a_resourceContext, const FrameResource& a_frameResource, const Renderer& a_renderer);

		void EndFrame(const Renderer& a_renderer) const;
		
		nlohmann::json Serialize() const;

		void AddPass(const std::shared_ptr<RenderGraphPassBase>& a_pass);

		template <Concept::IsDerivedRenderGraphPassBaseConcept PassType>
		std::weak_ptr<PassType> FindVALPass() const
		{
			const auto& l_itr = m_passMap.find(PassType::GetREFTypeINFO().k_staticTypeID);

			if (l_itr == m_passMap.end()) { return std::weak_ptr<PassType>(); }

			const auto& l_pass = l_itr->second.lock();

			if (!l_pass) { return std::weak_ptr<PassType>(); }

			return std::static_pointer_cast<PassType>(l_pass);
		}

		const auto& GetREFPassList() const { return m_passList; }

	private:

		void ExecutePassList(const ResourceContext& a_resourceContext, const FrameResource& a_frameResource, const Renderer& a_renderer) const;

		void ClearBackBuffer(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const;

		void TransitionBackBufferResource(const DirectCommandList&    a_directCommandList, 
									      const D3D12_RESOURCE_STATES a_beforeState,
										  const D3D12_RESOURCE_STATES a_afterState, 
										  const Struct::BackBuffer&	  a_backBuffer) const;

		void RemoveExpiredPassList();
		void RemoveExpiredPassMap ();

		static constexpr TypeAlias::Math::Color k_backBufferClearColor =
		{
			1.0F,
			0.80F,
			1.0F,
			1.0F
		};

		PassMap m_passMap = {};

		std::vector<std::shared_ptr<RenderGraphPassBase>> m_passList = {};

		Converter::RenderGraphJsonConverter m_jsonConverter = {};
	};
}