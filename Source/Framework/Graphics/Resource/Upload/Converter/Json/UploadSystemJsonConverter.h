#pragma once

namespace FWK::Graphics
{
	class UploadSystem;
}

namespace FWK::Converter
{
	class UploadSystemJsonConverter
	{
	public:

		 UploadSystemJsonConverter() = default;
		~UploadSystemJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::UploadSystem& a_uploadSystem) const;

		nlohmann::json Serialize(const Graphics::UploadSystem& a_uploadSystem) const;

	private:

		static constexpr std::string_view k_copyCommandAllocatorListJsonKey      = "CopyCommandAllocatorList";
		static constexpr std::string_view k_copyCommandAllocatorListCountJsonKey = "Count";

		static constexpr std::size_t k_defaultCopyCommandAllocatorListCount = 4ULL;
		static constexpr std::size_t k_emptyCopyCommandAllocatorListCount   = 0ULL;

		void DeserializeCopyCommandAllocator(const nlohmann::json& a_rootJson, Graphics::UploadSystem& a_uploadSystem) const;

		nlohmann::json SerializeCopyCommandAllocator(const Graphics::UploadSystem& a_uploadSystem) const;
	};
}