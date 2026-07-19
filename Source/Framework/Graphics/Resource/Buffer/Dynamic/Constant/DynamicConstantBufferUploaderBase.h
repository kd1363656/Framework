#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType>
	class DynamicConstantBufferUploaderBase : public DynamicBufferUploaderBase
	{
	public:

		explicit DynamicConstantBufferUploaderBase(const bool a_shouldAdvanceWritePosition) :
			DynamicBufferUploaderBase(sizeof(ConstantBufferType), a_shouldAdvanceWritePosition)
		{}
		~DynamicConstantBufferUploaderBase() override = default;

		bool Create(const Device& a_device) override
		{
			FWK_ASSERT_RETURN_VALUE_IF(!CreateUploadBuffer(a_device, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), "定数バッファ用UploadBufferの作成に失敗しました。", false);

			return true;
		}

		D3D12_GPU_VIRTUAL_ADDRESS Write(const ConstantBufferType& a_constantBuffer)
		{
			const std::span<const ConstantBufferType> l_constantBufferRange = { &a_constantBuffer, k_singleElementCount };

			return WriteElementRange(l_constantBufferRange);
		}

	private:

		static constexpr std::size_t k_singleConstantBufferCount = 1ULL;
	};
}