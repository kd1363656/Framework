#pragma once

namespace FWK::Graphics
{
	class UploadBuffer final
	{
	public:

		 UploadBuffer();
		~UploadBuffer();

		UploadBuffer(const UploadBuffer&) = delete;
		UploadBuffer(	   UploadBuffer&& a_other) noexcept;

		UploadBuffer& operator=(const UploadBuffer&) = delete;
		UploadBuffer& operator=(	  UploadBuffer&& a_other) noexcept;

		bool Create(const Device& a_device, const UINT64& a_bufferSize);

		void Release();

		std::uint8_t* FetchPTRMappedData() const;

		D3D12_GPU_VIRTUAL_ADDRESS FetchVALGPUVirtualAddress() const;

		const auto& GetREFUploadBuffer() const { return m_uploadBuffer; }

	private:

		bool Map  ();
		void UnMap();

		static constexpr D3D12_GPU_VIRTUAL_ADDRESS k_invalidGPUVirtualAddress = 0ULL;

		TypeAlias::ComPtr<ID3D12Resource2> m_uploadBuffer;

		std::uint8_t* m_mappedData;
	};
}