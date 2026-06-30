#pragma once

namespace FWK::Graphics
{
	class DynamicBufferUploaderBase
	{
	public:

		explicit DynamicBufferUploaderBase(const UINT64& a_typeSize);
		virtual ~DynamicBufferUploaderBase();

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create	    (const Device&		   a_device);

		void BeginFrame();
		
		nlohmann::json Serialize() const;
		
		void SetCreateCount(const UINT64& a_set) { m_createCount = a_set; }

		static constexpr auto& GetREFInvalidCreateCount() { return k_invalidCreateCount; }

		const auto& GetREFCreateCount() const { return m_createCount; }

	private:

		static constexpr UINT64 k_invalidCreateCount = 0ULL;
		static constexpr UINT64 k_initialOffset      = 0ULL;

		const UINT64 k_typeSize;

		UploadBuffer m_uploadBuffer;

		UINT64 m_createCount;

		UINT64 m_currentOffset;

		FWK_DEFINE_TYPE_INFO_ROOT(DynamicBufferUploaderBase)
	};
}