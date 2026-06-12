#pragma once

namespace FWK::Graphics
{
	class RenderGraphResourceStateTracker final
	{
	public:

		 RenderGraphResourceStateTracker() = default;
		~RenderGraphResourceStateTracker() = default;

		void Init();

		void BeginFrame();

	private:

	};
}