#pragma once

namespace FWK::Graphics
{
	class UploadSystem
	{
	public:
		
		 UploadSystem() = default;
		~UploadSystem() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create     (const Device&		   a_device);

		nlohmann::json Serialize() const;

		void AddCommandAllocator(const std::shared_ptr<CopyCommandAllocator>& a_copyCommandAllocator);

		const auto& GetREFCopyCommandAllocatorList() const { return m_copyCommandAllocatorList; }

	private:

		std::weak_ptr<CopyCommandAllocator> FetchMutablePTRCopyCommandAllocator();

		static constexpr std::size_t k_initialCurrentCopyCommandAllocatorIndex = 0ULL;
		static constexpr std::size_t k_copyCommandAllocatorIndexIncrement      = 1ULL;

		std::vector<std::shared_ptr<CopyCommandAllocator>> m_copyCommandAllocatorList = {};

		CopyCommandQueue m_copyCommandQueue = {};
		CopyCommandList  m_copyCommandList  = {};

		Converter::UploadSystemJsonConverter m_jsonConverter = {};

		std::size_t m_currentCopyCommandAllocatorIndex = k_initialCurrentCopyCommandAllocatorIndex;
	};
}