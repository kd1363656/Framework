#pragma once

namespace FWK::Graphics
{
	class PassTextureBase final
	{
	public:

		 PassTextureBase() = default;
		~PassTextureBase() = default;

		PassTextureBase(const PassTextureBase&)			  = delete;
		PassTextureBase(	  PassTextureBase&&) noexcept = default;
		
		PassTextureBase& operator=(const PassTextureBase&)			 = delete;
		PassTextureBase& operator=(		 PassTextureBase&&) noexcept = default;

		
		void SetRenderGraphResourceType(const Enum::RenderGraphResourceType a_set) { m_renderGraphResourceType = a_set; }

		void SetWidth (const UINT a_set) { m_width  = a_set; }
		void SetHeight(const UINT a_set) { m_height = a_set; }

		void SetIsFixedSize(const bool a_set) { m_isFixedSize = a_set; }
		
		auto GetVALRenderGraphResourceType() const { return m_renderGraphResourceType; }

		auto GetVALWidth () const { return m_width; }
		auto GetVALHeight() const { return m_height; }

		bool GetVALIsFixedSize() const { return m_isFixedSize; }
		
	protected:



	private:

		Enum::RenderGraphResourceType m_renderGraphResourceType = Enum::RenderGraphResourceType::Invalid;

		UINT m_width  = Constant::k_emptyTextureWidth;
		UINT m_height = Constant::k_emptyTextureHeight;

		bool m_isFixedSize = false;
	};
}