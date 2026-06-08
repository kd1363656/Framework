#pragma once

namespace FWK::Graphics
{
	class TextureSystem;
}

namespace FWK::Graphics
{
	class UploadSystem
	{
	public:
		
		 UploadSystem() = default;
		~UploadSystem() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create     (const Device&		   a_device);

		void SubmitPendingTextureCopyBatchIfNeededAndWait(TextureSystem& a_textureSystem);

		nlohmann::json Serialize() const;

		void AddCommandAllocator(const std::shared_ptr<CopyCommandAllocator>& a_copyCommandAllocator);

		const auto& GetREFCopyCommandAllocatorList() const { return m_copyCommandAllocatorList; }

	private:

		void RecordTextureCopy(const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& a_layoutList, const TypeAlias::ComPtr<ID3D12Resource2>& a_textureResource, const TypeAlias::ComPtr<ID3D12Resource2>& a_uploadBuffer) const;

		std::weak_ptr<CopyCommandAllocator> FetchMutablePTRCopyCommandAllocator();

		static constexpr std::size_t k_initialCurrentCopyCommandAllocatorIndex = 0ULL;
		static constexpr std::size_t k_copyCommandAllocatorIndexIncrement      = 1ULL;

		static constexpr UINT k_defaultTextureCopyDestinationX = 0U;
		static constexpr UINT k_defaultTextureCopyDestinationY = 0U;
		static constexpr UINT k_defaultTextureCopyDestinationZ = 0U;

		std::vector<std::shared_ptr<CopyCommandAllocator>> m_copyCommandAllocatorList = {};

		CopyCommandQueue m_copyCommandQueue = {};
		CopyCommandList  m_copyCommandList  = {};

		Converter::UploadSystemJsonConverter m_jsonConverter = {};

		std::size_t m_currentCopyCommandAllocatorIndex = k_initialCurrentCopyCommandAllocatorIndex;
	};
}