#pragma once

namespace FWK::Graphics
{
	class PassTextureBase
	{
	public:

				 PassTextureBase() = default;
		virtual ~PassTextureBase() = default;

		PassTextureBase(const PassTextureBase&)			  = delete;
		PassTextureBase(	  PassTextureBase&&) noexcept = default;
		
		PassTextureBase& operator=(const PassTextureBase&)			 = delete;
		PassTextureBase& operator=(		 PassTextureBase&&) noexcept = default;

		void SetWidth (const UINT a_set) { m_width  = a_set; }
		void SetHeight(const UINT a_set) { m_height = a_set; }

		void SetIsFixedSize(const bool a_set) { m_isFixedSize = a_set; }
		
		auto GetVALWidth () const { return m_width; }
		auto GetVALHeight() const { return m_height; }

		bool GetVALIsFixedSize() const { return m_isFixedSize; }
		
	protected:

		UINT FetchVALPassTextureWidth (const UINT a_clientWidth)  const;
		UINT FetchVALPassTextureHeight(const UINT a_clientHeight) const;
		
	private:

		UINT m_width  = Converter::TextureBinaryConverter::k_emptyTextureWidth;
		UINT m_height = Converter::TextureBinaryConverter::k_emptyTextureHeight;

		bool m_isFixedSize = false;
	};
}