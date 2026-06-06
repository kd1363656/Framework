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
		bool PostLoadCONFIG(const Window& a_window);
		
		void BeginFrame();
		void EndFrame  ();

		void SaveCONFIG() const;

		void ProcessWindowResizeRequest(const Struct::WindowResizeRequest& a_windowResizeRequest);

		const auto& GetREFRenderer		 () const { return m_renderer; }
		const auto& GetREFResourceContext() const { return m_resourceContext; }

		auto& GetMutableREFRenderer		  () { return m_renderer; }
		auto& GetMutableREFResourceContext() { return m_resourceContext; }

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