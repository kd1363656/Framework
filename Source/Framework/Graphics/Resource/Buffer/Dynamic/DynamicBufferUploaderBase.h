#pragma once

namespace FWK::Graphics
{
	class DynamicBufferUploaderBase
	{
	public:

		explicit DynamicBufferUploaderBase(const UINT64& a_bufferTypeSize);
		virtual ~DynamicBufferUploaderBase();

		void		 Deserialize(const nlohmann::json& a_rootJson);
		virtual bool Create	    (const Device&		   a_device) = 0;

		void BeginFrame();
		
		nlohmann::json Serialize() const;
		
		void SetCreateCount(const UINT64& a_set) { m_createCount = a_set; }

		static constexpr auto& GetREFInvalidCreateCount() { return k_invalidCreateCount; }

		const auto& GetREFTypeSize() const { return k_typeSize; }

		const auto& GetREFCreateCount() const { return m_createCount; }

	protected:

		bool CreateUploadBuffer(const Device& a_device, const UINT64& a_alignedSize);

		UINT64 AllocateElementRange(const UINT64& a_elementCount);

	private:

		static constexpr UINT64 k_invalidCreateCount        = 0ULL;
		static constexpr UINT64 k_initialElementBufferIndex = 0ULL;
		static constexpr UINT64 k_invalidElementBufferIndex = std::numeric_limits<UINT>::max();

		const UINT64 k_typeSize;

		UploadBuffer m_uploadBuffer;

		Converter::DynamicBufferUploaderBaseJsonConverter m_jsonConverter = {};

		UINT64 m_createCount;

		UINT64 m_currentElementIndex;

		FWK_DEFINE_TYPE_INFO_ROOT(DynamicBufferUploaderBase)
	};
}