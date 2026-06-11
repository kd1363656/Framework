#pragma once

namespace FWK::Graphics
{
	class ConstantBufferUploaderBase
	{
	public:

		explicit ConstantBufferUploaderBase() = default;
		virtual ~ConstantBufferUploaderBase() = default;

		ConstantBufferUploaderBase(const ConstantBufferUploaderBase&)  = delete;
		ConstantBufferUploaderBase(		 ConstantBufferUploaderBase&&) = delete;

		ConstantBufferUploaderBase& operator=(const ConstantBufferUploaderBase&)  = delete;
		ConstantBufferUploaderBase& operator=(	    ConstantBufferUploaderBase&&) = delete;

		void Deserialize(const nlohmann::json& a_rootJson)
		{
			if (a_rootJson.is_null()) { return; }
		}
		bool Create(const Device& a_device)
		{
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_createConstantBufferNUM == k_invalidCreateConstantBufferNUM, "定数バッファの作成個数が0のため、作成処理に失敗しました。", false);

			// 送る定数バッファの型サイズを256バイトにアライメントする
			const auto& l_alignedTypeSize    = Utility::AlignUp(m_constantBufferTypeSize, k_constantBufferAlignment);
			const auto& l_constantBufferSize = m_createConstantBufferNUM * l_alignedTypeSize;
			
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_uploadBuffer.Create(a_device, l_constantBufferSize), "定数バッファの生成処理に失敗しました。", false);

			return true;
		}

		void BeginFrame()
		{
			m_currentBufferIndex = k_initialBufferIndex;
		}

		nlohmann::json Serialize() const
		{
			return {};
		}

		std::size_t AllocateCurrentBufferIndex()
		{
			const auto l_currentBufferIndex = m_currentBufferIndex;

			++m_currentBufferIndex;

			return l_currentBufferIndex;
		}

	private:

		static constexpr UINT64 k_invalidCreateConstantBufferNUM = 0ULL;

		static constexpr UINT64 k_constantBufferAlignment = 256ULL;

		static constexpr std::size_t k_initialBufferIndex = 0ULL;

		UploadBuffer m_uploadBuffer = {};

		UINT64 m_createConstantBufferNUM = k_invalidCreateConstantBufferNUM;

		std::size_t m_currentBufferIndex = k_initialBufferIndex;

		FWK_DEFINE_TYPE_INFO_ROOT(ConstantBufferUploaderBase)
	};
}