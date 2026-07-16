#pragma once

namespace FWK::Graphics
{
	class ComputeRequestPerObjectBase
	{
	public:

		         ComputeRequestPerObjectBase() = default;
		virtual ~ComputeRequestPerObjectBase() = default;

		virtual void BeginFrame() = 0;

	protected:

		// ComputeShaderで使用するPerObject定数バッファを設定する
		template <Concept::IsDerivedDynamicBufferUploaderBaseConcept ConstantBufferUploaderType, typename ConstantBufferType>
		void SetupConstantBuffer(const ConstantBufferType&	   a_constantBuffer, 
								 const RootSignature&	       a_rootSignature,
								 const ComputeCommandList&     a_computeCommandList,
								 const FrameResource&	       a_frameResource,
								 const Enum::RootParameterType a_rootParameterType)
		{
			auto l_constantBufferUploader = a_frameResource.FindPTRDynamicBufferUploader<ConstantBufferUploaderType>().lock();

			FWK_ASSERT_RETURN_IF(!l_constantBufferUploader,												   "PerObject定数バッファアップローダーが取得できないため、定数バッファのセットに失敗しました。");
			FWK_ASSERT_RETURN_IF(l_constantBufferUploader->GetREFTypeSize() != sizeof(ConstantBufferType), "取得した定数バッファアップローダーの型サイズとGPU転送予定の定数バッファが一致しないため、定数バッファのセットに失敗しました。");

			const auto& l_gpuVirtualAddress = l_constantBufferUploader->WritePerObject(a_constantBuffer);

			// SetGraphicsRootConstantBufferView(ルートパラメータ番号、
			//									 CBVとして参照させるGPU仮想アドレス);
			// SetupConstantBufferView内でRootParameterTagからルートパラメータ番号を取得し、
			// 指定したRootParameterへUploadBuffer上の定数バッファを結びつける
			a_computeCommandList.SetupConstantBufferView(l_gpuVirtualAddress, a_rootSignature, a_rootParameterType);
		}

		FWK_DEFINE_TYPE_INFO_ROOT(ComputeRequestPerObjectBase)
	};
}