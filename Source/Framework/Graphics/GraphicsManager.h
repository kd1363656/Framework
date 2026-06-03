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
		bool PostLoadCONFIG();
		
	private:
		
#if defined(_DEBUG)
		bool EnableDebugLayer() const;
#endif

		Factory			m_factory		  = {};
		Device			m_device		  = {};
		ResourceContext m_resourceContext = {};
	};
}