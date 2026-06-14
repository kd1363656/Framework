#pragma once

namespace FWK::Graphics
{
	class TextureRecord final : public AssetRecordBase
	{
	public:

		 TextureRecord()		  = default;
		~TextureRecord() override = default;

		TextureRecord(const TextureRecord&)			  = delete;
		TextureRecord(	    TextureRecord&&) noexcept = default;

		TextureRecord& operator=(const TextureRecord&)			 = delete;
		TextureRecord& operator=(	   TextureRecord&&) noexcept = default;

		bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;
		
		void SetGPUResource(Struct::GPUResource&& a_set) { m_gpuResource = std::move(a_set); }

		void SetCurrentState(const D3D12_RESOURCE_STATES a_set) { m_currentState = a_set; }

		void SetSRVDescriptorIndex(const TypeAlias::StorageID a_set) { m_srvDescriptorIndex = a_set; }

		const Struct::GPUResource& GetREFGPUResource() const { return m_gpuResource; }

		D3D12_RESOURCE_STATES GetVALCurrentState() const { return m_currentState; }

		TypeAlias::StorageID GetVALSRVDescriptorIndex() const { return m_srvDescriptorIndex; }

	private:

		Struct::GPUResource m_gpuResource = {};
		
		D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COPY_DEST;

		TypeAlias::DescriptorIndex m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;
	};
}