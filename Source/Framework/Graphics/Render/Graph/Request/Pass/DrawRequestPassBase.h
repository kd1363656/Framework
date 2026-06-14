#pragma once

namespace FWK::Graphics
{
	class DrawRequestPassBase
	{
	public:

				 DrawRequestPassBase() = default;
		virtual ~DrawRequestPassBase() = default;

		virtual bool SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource) = 0;

	protected:

		// 定数バッファの上書きを許可
		template <Concept::IsDerivedConstantBufferUploaderBaseConcept ConstantBufferUploaderType, typename ConstantBufferType>
		void SetupConstantBuffer(const RootSignature&	       a_rootSignature,
								 const DirectCommandList&      a_directCommandList,
								 const FrameResource&	       a_frameResource,
								 const ConstantBufferType&     a_constantBuffer,
							     const Enum::RootParameterType a_rootParameterType)
		{
			auto l_constantBufferUploader = a_frameResource.FindPTRConstantBufferUploader<ConstantBufferUploaderType>().lock();

			FWK_ASSERT_RETURN_IF_FAILED(!l_constantBufferUploader,																"共通パス定数バッファアップローダーが取得できないため、定数バッファのセットに失敗しました。");
			FWK_ASSERT_RETURN_IF_FAILED(l_constantBufferUploader->GetREFConstantBufferTypeSize() != sizeof(ConstantBufferType), "取得した定数バッファアップローダーの型サイズとGPU転送予定の定数バッファが一致しないため、定数バッファのセットに失敗しました。");

			const auto& l_gpuVirtualAddress = l_constantBufferUploader->WriteCommonPass(a_constantBuffer);

			// SetupConstantBufferView内でRootParameterTagからルートパラメータ番号を取得し、
			// 指定したRootParameterへUploadBuffer上の定数バッファを結びつける
			// SetGraphicsRootConstantBufferView(ルートパラメータ番号、
			//									 CBVとして参照させるGPU仮想アドレス);
			a_directCommandList.SetupConstantBufferView(l_gpuVirtualAddress, a_rootSignature, a_rootParameterType);
		}

		FWK_DEFINE_TYPE_INFO_ROOT(DrawRequestPassBase)
	};
}