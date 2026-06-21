#pragma once

namespace FWK::Grpahics
{
	class RenderGraphResourceClearer final
	{
	public:

		 RenderGraphResourceClearer() = default;
		~RenderGraphResourceClearer() = default;

		RenderGraphResourceClearer(const RenderGraphResourceClearer&)			= delete;
		RenderGraphResourceClearer(	     RenderGraphResourceClearer&&) noexcept = default;
		
		RenderGraphResourceClearer& operator=(const RenderGraphResourceClearer&)		   = delete;
		RenderGraphResourceClearer& operator=(		RenderGraphResourceClearer&&) noexcept = default;

	private:

	};
}