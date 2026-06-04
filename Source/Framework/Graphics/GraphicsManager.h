#pragma once

namespace FWK::Graphics
{
	class GraphicsManager final : public FWK::Utility::SingletonBase<GraphicsManager>
	{
	private:

		friend class SingletonBase<GraphicsManager>;

		 GraphicsManager()          = default;
		~GraphicsManager() override = default;

	public:

		void INIT		   ();
		void LoadCONFIG	   ();
		bool PostLoadCONFIG();
		
		void BeginFrame();
		void EndFrame  ();

		void SaveCONFIG() const;

		const auto& GetREFRenderer() const { return m_renderer; }

		auto& GetMutableREFRenderer() { return m_renderer; }

	private:
		
#if defined(_DEBUG)
		bool EnableDebugLayer() const;
#endif

		const std::filesystem::path k_configFileIOPath = "Asset/Data/CONFIG/Graphics/GraphicsCONFIG.json";

		Factory			m_factory		  = {};
		Device			m_device		  = {};
		ResourceContext m_resourceContext = {};
		Renderer		m_renderer		  = {};

		Converter::GraphicsManagerJsonConverter m_graphicsManagerJsonConverter = {};
	};
}