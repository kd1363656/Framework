#pragma once

namespace FWK::Graphics
{
	class UploadBuffer final
	{
	public:

		 UploadBuffer();
		~UploadBuffer();

		UploadBuffer(const UploadBuffer&)	        = delete;
		UploadBuffer(	   UploadBuffer&&) noexcept = delete;

		UploadBuffer& operator=(const UploadBuffer&)		   = delete;
		UploadBuffer& operator=(	  UploadBuffer&&) noexcept = delete;

		bool Create(const Device& a_device, const UINT64& a_bufferSize);

		void Release();

		std::uint8_t* FetchPTRMappedData() const;

		const auto& GetREFUploadBuffer() const { return m_uploadBuffer; }

	private:

		bool Map  ();
		void UnMap();

		TypeAlias::ComPtr<ID3D12Resource2> m_uploadBuffer;

		std::uint8_t* m_mappedData;
	};
}