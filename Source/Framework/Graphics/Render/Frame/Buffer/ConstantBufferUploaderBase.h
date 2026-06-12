#pragma once

namespace FWK::Graphics
{
	class ConstantBufferUploaderBase
	{
	public:

		explicit ConstantBufferUploaderBase(const UINT64& a_constantBufferTypeSize);
		virtual ~ConstantBufferUploaderBase();

		ConstantBufferUploaderBase(const ConstantBufferUploaderBase&)  = delete;
		ConstantBufferUploaderBase(		 ConstantBufferUploaderBase&&) = delete;

		ConstantBufferUploaderBase& operator=(const ConstantBufferUploaderBase&)  = delete;
		ConstantBufferUploaderBase& operator=(	    ConstantBufferUploaderBase&&) = delete;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create	    (const Device&		   a_device);

		void BeginFrame();
		
		nlohmann::json Serialize() const;
		
		void SetCreateConstantBufferCount(const UINT64& a_set) { m_createConstantBufferCount = a_set; }

		static constexpr auto& GetREFInvalidCreateConstantBufferCount() { return k_invalidCreateConstantBufferCount; }

		const auto& GetREFConstantBufferTypeSize() const { return m_constantBufferTypeSize; }

		const auto& GetREFCreateConstantBufferCount() const { return m_createConstantBufferCount; }

	protected:

		// 定数バッファの上書きを許さない場合に使用
		// 仮想アドレスのインデックスがこの関数を呼び出すたびに代わるから
		template <typename ConstantBufferType>
		D3D12_GPU_VIRTUAL_ADDRESS WritePerObject(const ConstantBufferType& a_constantBuffer)
		{
			const auto l_currentBufferIndex = AllocateCurrentBufferIndex();

			return Write(a_constantBuffer, l_currentBufferIndex);
		}

		// 定数バッファの上書きをしてもよいものに使用
		// カメラやディレクショナルライトと言った単一のものに主に使用
		template <typename ConstantBufferType>
		D3D12_GPU_VIRTUAL_ADDRESS WriteCommonPass(const ConstantBufferType& a_constantBuffer)
		{
			return Write(a_constantBuffer, k_initialBufferIndex);
		}

		UINT64 AllocateCurrentBufferIndex();

	private:

		template <typename ConstantBufferType>
		D3D12_GPU_VIRTUAL_ADDRESS Write(const ConstantBufferType& a_constantBuffer, const UINT64& a_writeIndex) const
		{
			const auto l_alignedConstantBufferSize = Utility::AlignUp(sizeof(ConstantBufferType), k_constantBufferAlignment);

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_alignedConstantBufferSize != Utility::AlignUp(m_constantBufferTypeSize, k_constantBufferAlignment), "ConstantBufferUploaderの型サイズが一致しておらず、定数バッファ書き込み処理に失敗しました。", {});
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_writeIndex >= m_createConstantBufferCount,															"ConstantBufferUploaderの容量を超えており、定数バッファ書き込み処理に失敗しました。",		  {});

			auto* const l_mappedData = m_uploadBuffer.FetchPTRMappedData();

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_mappedData, "ConstantBufferUploaderの書き込み先取得に失敗しており、定数バッファ書き込み処理に失敗しました。", {});

			// 書き込み先のインデックスとサイズ分のオフセットを計算
			const auto l_writeOffset = a_writeIndex * l_alignedConstantBufferSize;

			// オフセット位置にある定数バッファに書き込む
			std::memcpy(l_mappedData + l_writeOffset, &a_constantBuffer, sizeof(ConstantBufferType));

			// 仮想アドレスからオフセット位置分ずらしたものを返す
			return m_uploadBuffer.FetchVALGPUVirtualAddress() + l_writeOffset;
		}

		static constexpr UINT64 k_invalidCreateConstantBufferCount = 0ULL;

		static constexpr UINT64 k_constantBufferAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

		static constexpr UINT64 k_initialBufferIndex = 0ULL;

		UploadBuffer m_uploadBuffer;

		Converter::ConstantBufferUploaderBaseJsonConverter m_jsonConverter = {};

		UINT64 m_constantBufferTypeSize;

		UINT64 m_createConstantBufferCount;

		UINT64 m_currentBufferIndex;

		FWK_DEFINE_TYPE_INFO_ROOT(ConstantBufferUploaderBase)
	};
}