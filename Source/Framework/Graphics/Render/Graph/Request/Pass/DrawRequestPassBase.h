#pragma once

namespace FWK::Graphics
{
	class DrawRequestPassBase
	{
	public:

				 DrawRequestPassBase() = default;
		virtual ~DrawRequestPassBase() = default;

		virtual void BeginFrame() { /*必要に応じてオーバーライドしてください*/ };

		virtual bool SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource) = 0;

	protected:

		// 定数バッファの上書きを許可
		template <Concept::IsDerivedConstantBufferUploaderBaseConcept ConstantBufferUploaderType, typename ConstantBufferType>
		bool SetupConstantBuffer(const RootSignature&	       a_rootSignature,
								 const DirectCommandList&      a_directCommandList,
								 const FrameResource&	       a_frameResource,
								 const ConstantBufferType&     a_constantBuffer,
							     const Enum::RootParameterType a_rootParameterType)
		{
			auto l_constantBufferUploader = a_frameResource.FindPTRConstantBufferUploader<ConstantBufferUploaderType>().lock();

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_constantBufferUploader, "共通パス定数バッファが取得できないため、描画申請処理に失敗しました。", false);

			const auto& l_gpuVirtualAddress = l_constantBufferUploader->WriteCommonPass(a_constantBuffer);

			// SetupConstantBufferView内でRootParameterTagからルートパラメータ番号を取得し、
			// 指定したRootParameterへUploadBuffer上の定数バッファを結びつける
			// SetGraphicsRootConstantBufferView(ルートパラメータ番号、
			//									 CBVとして参照させるGPU仮想アドレス);
			a_directCommandList.SetupConstantBufferView(l_gpuVirtualAddress, a_rootSignature, a_rootParameterType);

			return true;
		}

		FWK_DEFINE_TYPE_INFO_ROOT(DrawRequestPassBase)
	};
}