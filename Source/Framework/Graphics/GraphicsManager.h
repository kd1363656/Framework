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

		void INIT();
		
	private:
		
#if defined(_DEBUG)
		bool EnableDebugLayer() const;
#endif

	};
}