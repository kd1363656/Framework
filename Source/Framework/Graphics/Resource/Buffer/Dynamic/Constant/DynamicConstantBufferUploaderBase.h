#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType>
	class DynamicConstantBufferUploaderBase : public DynamicBufferUploaderBase
	{
	public:

		DynamicConstantBufferUploaderBase() :
			DynamicBufferUploaderBase(sizeof(ConstantBufferType))
		{}
		~DynamicConstantBufferUploaderBase() override = default;

		bool Create(const Device& a_device) override
		{
			FWK_ASSERT_RETURN_VALUE_IF(!CreateUploadBuffer(a_device, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), "定数バッファ用UploadBufferの作成に失敗しました。", false);

			return true;
		}

		// 定数バッファの上書きを許さない場合に使用
		// 仮想アドレスのインデックスがこの関数を呼び出すたびに代わるから
		template <typename ConstantBufferType>
		D3D12_GPU_VIRTUAL_ADDRESS WritePerObject(const ConstantBufferType& a_constantBuffer)
		{
			return WriteElementAndAdvance(a_constantBuffer);
		}

		// 定数バッファの上書きをしてもよいものに使用
		// カメラやディレクショナルライトと言った単一のものに主に使用
		template <typename ConstantBufferType>
		D3D12_GPU_VIRTUAL_ADDRESS WriteCommonPass(const ConstantBufferType& a_constantBuffer)
		{
			return WriteElementAt(a_constantBuffer, k_commonPassElementIndex);
		}

	private:

		static constexpr UINT64 k_commonPassElementIndex = 0ULL;
	};
}